#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
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
            printf("Node %d received a message.\n", node_id);
            if (msg.type == MSG_SHUTDOWN)
            {
                size_t pass_shutdown_bytes = write(ring_write_fd, &msg, sizeof(RingMessage));
                if (pass_shutdown_bytes == -1)
                {
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
                // Pass the token to the next node
                msg.hop_count++; // Increment hop count for the token
                size_t bytes_written = write(ring_write_fd, &msg, sizeof(RingMessage));
                if (bytes_written == -1)
                {
                    perror("write on ring");
                    exit(1); // Exit on write error
                }
                printf("Node %d passed the token to the next node.\n", node_id);
                sleep(1);
            }
        }
        // Done passing the token
        char result_msg[MAX_PAYLOAD];
        snprintf(result_msg, MAX_PAYLOAD, "Node %d passed the token successfully.", node_id);
        RingMessage msg_to_parent = make_report_msg(node_id, STATUS_OK, result_msg);
        size_t bytes_written = write(stat_write_fd, &msg_to_parent, sizeof(RingMessage));
        if (bytes_written == -1)
        {
            perror("write to parent");
            exit(1); // Exit on write error
        }
    }
}
