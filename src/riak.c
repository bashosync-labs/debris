/*********************************************************************
 *
 * riak_types.h: Riak Operations
 *
 * Copyright (c) 2007-2013 Basho Technologies, Inc.  All Rights Reserved.
 *
 * This file is provided to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License.  You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *********************************************************************/

#include "riak.h"
#include "riak_pb_message.h"
#include "riak_kv.pb-c.h"
#include "riak_utils.h"
#include "user_call_backs.h"

static riak_error
riak_synchronous_request(riak_event      *rev,
                         riak_pb_message *request,
                         void           **response) {
    riak_event_set_response_cb(rev, (riak_response_callback)riak_sync_cb);
    riak_sync_wrapper wrapper;
    wrapper.rev = rev;
    riak_event_set_cb_data(rev, &wrapper);

    riak_error err = riak_send_req(rev, request);
    if (err) {
        riak_log(rev, RIAK_LOG_FATAL, "Could not send request");
        return err;
    }

    // Terminates only on error or timeout
    event_base_dispatch(rev->base);

    *response = wrapper.response;

    return ERIAK_OK;
}

riak_error
riak_ping(riak_context  *ctx) {
    riak_event *rev = riak_event_new(ctx, NULL, NULL, NULL, NULL);
    if (rev == NULL) {
        return ERIAK_EVENT;
    }
    riak_pb_message    *request  = NULL;
    riak_ping_response *response = NULL;
    riak_error err = riak_encode_ping_request(rev, &request);
    if (err) {
        return err;
    }
    err = riak_synchronous_request(rev, request, (void**)&response);
    if (err) {
        return err;
    }
    if (response->success != RIAK_TRUE) {
        return ERIAK_NO_PING;
    }
    return ERIAK_OK;
}


riak_error
riak_get(riak_context      *ctx,
         riak_binary       *bucket,
         riak_binary       *key,
         riak_get_options  *opts,
         riak_get_response **response) {
    riak_event *rev = riak_event_new(ctx, NULL, NULL, NULL, NULL);
    if (rev == NULL) {
        return ERIAK_EVENT;
    }
    riak_pb_message   *request  = NULL;
    riak_error err = riak_encode_get_request(rev, bucket, key, opts, &request);
    if (err) {
        return err;
    }
    err = riak_synchronous_request(rev, request, (void**)response);
    if (err) {
        return err;
    }

    return ERIAK_OK;
}

riak_error
riak_put(riak_context      *ctx,
         riak_object       *obj,
         riak_put_options  *opts,
         riak_put_response **response) {
    riak_event *rev = riak_event_new(ctx, NULL, NULL, NULL, NULL);
    if (rev == NULL) {
        return ERIAK_EVENT;
    }
    riak_pb_message *request = NULL;

    riak_error err = riak_encode_put_request(rev, obj, opts, &request);
    if (err) {
        return err;
    }
    err = riak_synchronous_request(rev, request, (void**)response);
    if (err) {
        return err;
    }

    return ERIAK_OK;
}

riak_error
riak_delete(riak_context      *ctx,
         riak_binary          *bucket,
         riak_binary          *key,
         riak_delete_options  *opts) {
    riak_event *rev = riak_event_new(ctx, NULL, NULL, NULL, NULL);
    if (rev == NULL) {
        return ERIAK_EVENT;
    }
    riak_pb_message *request  = NULL;
    riak_error err = riak_encode_delete_request(rev, bucket, key, opts, &request);
    if (err) {
        return err;
    }
    riak_get_response *response = NULL;
    err = riak_synchronous_request(rev, request, (void**)&response);
    if (err) {
        return err;
    }

    return ERIAK_OK;
}
