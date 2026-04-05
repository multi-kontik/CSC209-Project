#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ring.h"

/*
 * Creates a result message to send back to the parent after processing a task.
 * The parent uses task_id and sequence_num to match this result back to the task
 * it originally injected into the ring.
 */
RingMessage make_report_msg(int node_id, int task_id, int sequence_num, TaskStatus status, const char *result)
{
    RingMessage msg;
    msg.type = MSG_RESULT;        // This is a result message
    msg.sender_id = node_id;      // The ID of the node sending the result
    msg.receiver_id = -1;         // Send back to parent (could be set to specific node)
    msg.task_id = task_id;        // Preserved from the DATA message so the parent can match
    msg.sequence_num = sequence_num; // Preserved from the DATA message so the parent can match
    msg.hop_count = 0;            // Not relevant for result messages, but we can set it to 0
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
    msg.task_id = 0;
    msg.sequence_num = 0;
    msg.hop_count = 0;
    return msg;
}

/*
 * Creates a shutdown message for the parent to signal nodes to terminate gracefully.
 */
RingMessage make_shutdown_msg()
{
    RingMessage msg;
    msg.type = MSG_SHUTDOWN; // This is a shutdown message
    msg.sender_id = -1;      // The parent should be the sender of the shutdown message
    msg.receiver_id = -1;    // Shutdown can be processed by any node
    msg.task_id = 0;
    msg.sequence_num = 0;
    msg.hop_count = 0;
    return msg;
}

/*
 * Creates a DATA message for the parent to inject a task into the ring.
 * The payload carries a file path (not file contents) so the worker node
 * opens the file itself via fopen in jobs.c, per the team's design decision.
 */
RingMessage make_data_msg(int receiver_id, int task_id, int sequence_num, const char *filepath)
{
    RingMessage msg;
    msg.type = MSG_DATA;             // This is a task/data message
    msg.sender_id = -1;              // The parent is the sender
    msg.receiver_id = receiver_id;   // Which node should process this task
    msg.task_id = task_id;           // Unique task identifier for matching results
    msg.sequence_num = sequence_num; // Order this task was sent by parent
    msg.hop_count = 0;               // Starts at zero, incremented on each forward
    msg.status = STATUS_PENDING;     // Not yet processed
    strncpy(msg.payload, filepath, MAX_PAYLOAD);
    msg.payload[MAX_PAYLOAD - 1] = '\0'; // Null-termination
    msg.result[0] = '\0';                // No result yet
    return msg;
}