#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "ring.h"

#define NUM_NODES 5

int main()
{
    int ring_pipes[NUM_NODES][2]; // Pipes for communication between nodes
    int stat_pipes[NUM_NODES][2]; // Pipes nodes use to send results back to parent

    // Create pipes for each node
    for (int i = 0; i < NUM_NODES; i++)
    {
        if (pipe(ring_pipes[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        if (pipe(stat_pipes[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Fork child processes for each node
    for (int i = 0; i < NUM_NODES; i++)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // Child node i

            // Keep only the necessary pipe ends for this node
            int node_ring_read = ring_pipes[i][0];
            int node_ring_write = ring_pipes[(i + 1) % NUM_NODES][1];
            int node_stat_write = stat_pipes[i][1];

            // Close unused pipe ends
            for (int j = 0; j < NUM_NODES; j++)
            {
                // ring pipes
                if (ring_pipes[j][0] != node_ring_read)
                {
                    close(ring_pipes[j][0]);
                }
                if (ring_pipes[j][1] != node_ring_write)
                {
                    close(ring_pipes[j][1]);
                }

                // stat pipes
                close(stat_pipes[j][0]); // child never reads from stat pipes
                if (stat_pipes[j][1] != node_stat_write)
                {
                    close(stat_pipes[j][1]);
                }
            }

            run_node(i, node_ring_read, node_ring_write, node_stat_write);
            exit(0);
        }
    }

    // Parent: close ends it does not need
    for (int i = 0; i < NUM_NODES; i++)
    {
        close(ring_pipes[i][0]); // parent only writes to node 0
        if (i != 0)
        {
            close(ring_pipes[i][1]);
        }
        close(stat_pipes[i][1]); // parent only reads from stat pipes
    }

    // Send initial token to node 0
    RingMessage token_msg = make_token_msg();
    size_t bytes_written = write(ring_pipes[0][1], &token_msg, sizeof(RingMessage));
    if (bytes_written == -1)
    {
        perror("write to ring (initial token)");
        exit(EXIT_FAILURE);
    }
    printf("Parent sent initial token to node 0.\n");

    size_t total_reports = 0;
    while (total_reports < NUM_NODES * 2)
    {
        for (int i = 0; i < NUM_NODES; i++)
        {
            RingMessage report_msg;
            ssize_t bytes_read = read(stat_pipes[i][0], &report_msg, sizeof(RingMessage));
            if (bytes_read > 0)
            {
                printf("Parent received report from node %d: %s\n", report_msg.sender_id, report_msg.result);
                total_reports++;
            }
            else if (bytes_read == -1 && errno != EAGAIN)
            {
                perror("read from stat pipe");
                exit(EXIT_FAILURE);
            }
            sleep(5);
        }
    }
}