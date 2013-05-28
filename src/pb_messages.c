#include "pb_messages.h"
#include "pb_transport.h"
#include "riak.h"
#include "riak.pb-c.h"
#include "riak_kv.pb-c.h"

#include <stdio.h>
#include <stdlib.h>

int riak_pb_get(struct riak_pb_transport *pbtransport,
                struct riak_string *bucket,
                struct riak_string *key) {
   void *msgbuf;
   unsigned msglen;

   RpbGetReq getmsg = RPB_GET_REQ__INIT;

   getmsg.bucket.data = bucket->data;
   getmsg.bucket.len = bucket->len;

   getmsg.key.data = key->data;
   getmsg.key.len = key->len;

   msglen = rpb_get_req__get_packed_size (&getmsg);
   msgbuf = malloc (msglen);
   rpb_get_req__pack (&getmsg, msgbuf);
   printf("riak_get [%s|%s = %i bytes]\n", bucket->data, key->data, msglen);
   pbtransport->send_message(pbtransport->transport_data, MSG_RPBGETREQ, msgbuf, msglen);

   struct pb_response response;
   pbtransport->receive_message(pbtransport->transport_data, MSG_RPBGETRESP, &response);
   printf("Response = %s\n", (char*)response.buf);
   printf("Response length = %d\n", response.len);
   // decode the PB response etc
   RpbGetResp *getresp = rpb_get_resp__unpack(NULL, response.len, (char*)response.buf);
   printf("RPB msg %d\n",getresp->n_content);
   RpbContent *c = getresp->content[0];
   printf("Value=[%s]\n", c->value.data);
   free(response.buf);
}
