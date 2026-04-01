#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ring.h"

/*
 * Creates a result message to send back to the parent after processing a task.
 * For simplicity, this function just returns a success message. In a real implementation,
 * it would perform the actual task and populate the result accordingly.
 */
RingMessage make_report_msg(int node_id, TaskStatus status, const char *result)
{
    RingMessage msg;
    msg.type = MSG_RESULT;   // This is a result message
    msg.sender_id = node_id; // The ID of the node sending the result
    msg.receiver_id = -1;    // Send back to parent (could be set to specific node)
    msg.status = status;
    snprintf(msg.result, MAX_PAYLOAD, "%s", result);
    return msg;
}

/*
 * Creates a token message for parent to pass around the ring. The token doesn't carry any data,
 * but we can use the sender_id and hop_count for tracking purposes.
 */
RingMessage make_token_msg()
{
    RingMessage msg;
    msg.type = MSG_TOKEN; // This is a token message
    msg.sender_id = -1;   // The parent should be the sender of the token once it's created
    msg.receiver_id = -1; // Token can be processed by any node
    return msg;
}