// (C) 2018 University of Bristol. See License.txt


#include <fcntl.h>

#include "octetStream.h"

#include <string.h>
#include "Networking/sockets.h"
#include "Tools/sha1.h"
#include "Exceptions/Exceptions.h"
#include "Networking/data.h"
#include "Tools/int.h"

#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sodium.h>

void octetStream::clear()
{
    if (data)
        delete[] data;
    data = 0;
    len = mxlen = ptr = 0;
}

void octetStream::assign(const octetStream& os)
{
  if (os.len>=mxlen)
    {
      if (data)
        delete[] data;
      mxlen=os.mxlen;  
      data=new octet[mxlen];
    }
  len=os.len;
  memcpy(data,os.data,len*sizeof(octet));
  ptr=os.ptr;
}


void octetStream::swap(octetStream& os)
{
  const size_t size = sizeof(octetStream);
  char tmp[size];
  memcpy(tmp, this, size);
  memcpy(this, &os, size);
  memcpy(&os, tmp, size);
}


octetStream::octetStream(size_t maxlen)
{
  mxlen=maxlen; len=0; ptr=0;
  data=new octet[mxlen];
}


octetStream::octetStream(const octetStream& os)
{
  mxlen=os.mxlen;
  len=os.len;
  data=new octet[mxlen];
  memcpy(data,os.data,len*sizeof(octet));
  ptr=os.ptr;
}


void octetStream::hash(octetStream& output) const
{
	SHA256(data, len, output.data);
	output.len=16;
}


octetStream octetStream::hash() const
{
	octetStream h(16);
	hash(h);
	return h;
}


bool octetStream::equals(const octetStream& a) const
{
  if (len!=a.len) { return false; }
  for (size_t i=0; i<len; i++)
    { if (data[i]!=a.data[i]) { return false; } }
  return true;
}

void octetStream::append_random(size_t num)
{
  resize(len+num);
  RAND_bytes(data+len, num);
  len+=num;
}


void octetStream::concat(const octetStream& os)
{
  resize(len+os.len);
  memcpy(data+len,os.data,os.len*sizeof(octet));
  len+=os.len;
}


void octetStream::store_bytes(octet* x, const size_t l)
{
  resize(len+4+l); 
  encode_length(data+len,l,4); len+=4;
  memcpy(data+len,x,l*sizeof(octet));
  len+=l;
}

void octetStream::get_bytes(octet* ans, size_t& length)
{
  length=decode_length(data+ptr,4); ptr+=4;
  memcpy(ans,data+ptr,length*sizeof(octet));
  ptr+=length;
}

void octetStream::store_int(size_t l, int n_bytes)
{
  resize(len+n_bytes);
  encode_length(data+len,l,n_bytes);
  len+=n_bytes;
}

void octetStream::store(int l)
{
  resize(len+4);
  INT_TO_BYTES(data+len,l);
  len+=4;
}


size_t octetStream::get_int(int n_bytes)
{
  size_t res=decode_length(data+ptr,n_bytes);
  ptr+=n_bytes;
  return res;
}

void octetStream::get(int& l)
{
  l=BYTES_TO_INT(data+ptr);
  ptr+=4;
}


void octetStream::store(const vector<int>& v)
{
  store(v.size());
  for (int x : v)
    store(x);
}


void octetStream::get(vector<int>& v)
{
  size_t size;
  get(size);
  v.resize(size);
  for (int& x : v)
    get(x);
}


void octetStream::exchange(int send_socket, int receive_socket)
{
  send(send_socket, len, LENGTH_SIZE);
  size_t new_len;
  receive(receive_socket, new_len, LENGTH_SIZE);
  if (new_len > mxlen)
      resize_precise(max(new_len, len));
  const size_t buffer_size = 100000;
  size_t sent = 0, received = 0;
  while (sent < len or received < new_len)
    {
      if (sent < len)
        {
          size_t to_send = min(buffer_size, len - sent);
          send(send_socket, data + sent, to_send);
          sent += to_send;
        }
      if (received < new_len)
        {
          size_t to_receive = min(buffer_size, new_len - received);
          receive(receive_socket, data + received, to_receive);
          received += to_receive;
        }
    }
  len = new_len;
  reset_read_head();
}

// Construct the ciphertext as `crypto_secretbox(pt, counter||random)`
void octetStream::encrypt_sequence(const octet* key, uint64_t counter)
{
  octet nonce[crypto_secretbox_NONCEBYTES];
  int i;
  int message_len_bytes = len;
  RAND_bytes(nonce, sizeof nonce);
  if(counter == UINT64_MAX) {
      throw Processor_Error("Encryption would overflow counter. Too many messages.");
  } else {
      counter++;
  }
  for(i=0; i<8; i++) {
      nonce[i] = uint8_t ((counter >> (8*i)) & 0xFF);
  }

  resize(len + crypto_secretbox_MACBYTES + crypto_secretbox_NONCEBYTES);

  // Encrypt data in-place
  crypto_secretbox_easy(data, data, message_len_bytes, nonce, key);
  // Adjust length to account for MAC, then append nonce
  len += crypto_secretbox_MACBYTES;
  append(nonce, sizeof nonce);
}

void octetStream::decrypt_sequence(const octet* key, uint64_t counter)
{
  int ciphertext_len = len - crypto_box_NONCEBYTES;
  const octet *nonce = data + ciphertext_len;
  int i;
  uint64_t recvCounter=0;
  // Numbers are typically 24U + 16U so cast to int is safe.
  if (len < (int)(crypto_box_NONCEBYTES + crypto_secretbox_MACBYTES))
  {
    throw Processor_Error("Cannot decrypt octetStream: ciphertext too short");
  }
  for(i=7; i>=0; i--) {
      recvCounter |= ((uint64_t) *(nonce + i)) << (i*8);
  }
  if(recvCounter != counter + 1) {
      throw Processor_Error("Incorrect counter on stream.  Possible MITM.");
  }
  if (crypto_secretbox_open_easy(data, data, ciphertext_len, nonce, key) != 0)
  {
    throw Processor_Error("octetStream decryption failed!");
  }
  rewind_write_head(crypto_box_NONCEBYTES + crypto_secretbox_MACBYTES);
}

void octetStream::encrypt(const octet* key)
{
  octet nonce[crypto_secretbox_NONCEBYTES];
  randombytes_buf(nonce, sizeof nonce);
  int message_len_bytes = len;
  resize(len + crypto_secretbox_MACBYTES + crypto_secretbox_NONCEBYTES);

  // Encrypt data in-place
  crypto_secretbox_easy(data, data, message_len_bytes, nonce, key);
  // Adjust length to account for MAC, then append nonce
  len += crypto_secretbox_MACBYTES;
  append(nonce, sizeof nonce);
}

void octetStream::decrypt(const octet* key)
{
  int ciphertext_len = len - crypto_box_NONCEBYTES;
  // Numbers are typically 24U + 16U so cast to int is safe.
  if (len < (int)(crypto_box_NONCEBYTES + crypto_secretbox_MACBYTES))
  {
    throw Processor_Error("Cannot decrypt octetStream: ciphertext too short");
  }
  if (crypto_secretbox_open_easy(data, data, ciphertext_len, data + ciphertext_len, key) != 0)
  {
    throw Processor_Error("octetStream decryption failed!");
  }
  rewind_write_head(crypto_box_NONCEBYTES + crypto_secretbox_MACBYTES);
}

void octetStream::input(istream& s)
{
  size_t size;
  s.read((char*)&size, sizeof(size));
  resize_precise(size);
  s.read((char*)data, size);
}

void octetStream::output(ostream& s)
{
  s.write((char*)&len, sizeof(len));
  s.write((char*)data, len);
}

ostream& operator<<(ostream& s,const octetStream& o)
{
  for (size_t i=0; i<o.len; i++)
    { int t0=o.data[i]&15;
      int t1=o.data[i]>>4;
      s << hex << t1 << t0 << dec;
    }
  return s;
}




