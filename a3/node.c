#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "ring.h"

/*
 * Runs the node process: pass the token, process tasks, and send results back to the parent.
 * @node_id: The ID of this node
 * @ring_read_fd: File descriptor for reading from the ring
 * @ring_write_fd: File descriptor for writing to the ring
 * @stat_write_fd: File descriptor for writing status messages
 */
void run_node(int node_id, int ring_read_fd, int ring_write_fd, int stat_write_fd)
{
    printf("Node %d created.\n", node_id);

    RingMessage msg;
    char *result = malloc(MAX_PAYLOAD);
    while (1)
    {
        // Check for control messages from the parent
        size_t bytes_read = read(ring_read_fd, &msg, sizeof(RingMessage));
        if (bytes_read == -1)
        {
            if (errno == EINTR)
            {
                continue; // Interrupted by signal, retry
            }
            else
            {
                perror("read");
                exit(1); // Exit on read error
            }
        }
        else if (bytes_read > 0)
        {
            if (msg.type == MSG_SHUTDOWN)
            {
                size_t pass_shutdown_bytes = write(ring_write_fd, &msg, sizeof(RingMessage));
                printf("Node %d received shutdown message and is passing it to the next node.\n", node_id);
                if (pass_shutdown_bytes == -1)
                {
                    printf("Node %d failed to pass the shutdown message.\n", node_id);
                    perror("pass shutdown on ring");
                    exit(1); // Exit on write error
                }
                // Clean up and exit gracefully
                close(ring_read_fd);
                close(ring_write_fd);
                close(stat_write_fd);
                printf("Node %d shutting down.\n", node_id);
                exit(0);
            }
            else if (msg.type == MSG_TOKEN)
            {
                // If it is a free token
                printf("Node %d received a token.\n", node_id);
                // Pass the token to the next node
                msg.hop_count++; // Increment hop count for the token
                size_t bytes_written = write(ring_write_fd, &msg, sizeof(RingMessage));
                if (bytes_written == -1)
                {
                    printf("Node %d failed to pass the token.\n", node_id);
                    perror("write on ring");
                    exit(1); // Exit on write error
                }
                printf("Node %d passed the token to the next node.\n", node_id);
                snprintf(result, MAX_PAYLOAD, "Node %d passed the token.", node_id);
                sleep(1);
            }
            else if (msg.type == MSG_DATA && msg.receiver_id != node_id)
            {
                // If it is a task message for a different node, we should still pass it along
                msg.hop_count++; // Increment hop count for the message
                size_t bytes_written = write(ring_write_fd, &msg, sizeof(RingMessage));
                if (bytes_written == -1)
                {
                    printf("Node %d failed to pass the task message.\n", node_id);
                    perror("write on ring");
                    exit(1); // Exit on write error
                }
                printf("Node %d passed the task message to the next node.\n", node_id);
                sleep(1);
            }
            else if (msg.type == MSG_DATA && msg.receiver_id == node_id)
            {
                // If it is a task message for this node, process the task
                printf("Node %d has received a task and it's processing it.\n", node_id);

                // Find and perform the task
                char *ret = task(msg);
                // Error checking
                if (result == NULL)
                {
                    snprintf(result, MAX_PAYLOAD, "Node %d failed to process the task.", node_id);
                    msg.status = STATUS_ERROR;
                }
                else
                {
                    snprintf(result, MAX_PAYLOAD, "%s", ret);
                    msg.status = STATUS_OK;
                }

                // Send the report back to the parent
                RingMessage report_msg = make_report_msg(node_id, msg.task_id, msg.sequence_num, msg.status, result);
                size_t stat_bytes_written = write(stat_write_fd, &report_msg, sizeof(RingMessage));
                if (stat_bytes_written == -1)
                {
                    printf("Node %d failed to send the report to the parent.\n", node_id);
                    perror("write on stat pipe");
                    exit(1); // Exit on write error
                }
                printf("Node %d sent the report back to the parent.\n", node_id);
                sleep(1);
            }
        }
    }
}
