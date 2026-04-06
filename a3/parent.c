#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ring.h"

/*
 * A single task tracked by the parent. Updated as results come back.
 */
typedef struct
{
    int task_id;                // unique task identifier
    int receiver_id;            // which node should process this task
    int sequence_num;           // order this task was sent by parent
    char filepath[MAX_PAYLOAD]; // input file path (sent as payload)
    TaskStatus status;          // pending / ok / error
    char result[MAX_PAYLOAD];   // worker's result, filled in on reply
} TaskEntry;

// Task names, indexed by node_id (matches TaskType order)
static const char *task_names[] = {
    "Word Count",          // node 0
    "Longest Sentence",    // node 1
    "Avg Sentence Length", // node 2
    "Longest Word",        // node 3
    "Avg Word Length",     // node 4
    "Sentence Count",      // node 5
    "Section Count",       // node 6
    "Character Count"      // node 7
};

/*
 * Runs the parent orchestration: inject tasks into the ring, collect results
 * from the stat pipes, print a summary table, and send a shutdown message.
 * @ring_write_fd: write end of the ring pipe (feeds into node 0)
 * @stat_read_fds: array of parent's read ends, one per node
 * @num_nodes: number of nodes in the ring
 * @file_paths: input file paths (from argv)
 * @num_files: number of file paths
 */
void run_parent(int ring_write_fd, int stat_read_fds[], int num_nodes, const char *file_paths[], int num_files)
{
    // One task per (file, node) pair
    int total_tasks = num_nodes * num_files;
    TaskEntry tasks[total_tasks];

    // Build the task list
    int seq = 0;
    for (int f = 0; f < num_files; f++)
    {
        for (int n = 0; n < num_nodes; n++)
        {
            tasks[seq].task_id = seq;
            tasks[seq].receiver_id = n;
            tasks[seq].sequence_num = seq;
            strncpy(tasks[seq].filepath, file_paths[f], MAX_PAYLOAD);
            tasks[seq].filepath[MAX_PAYLOAD - 1] = '\0'; // Null-termination
            tasks[seq].status = STATUS_PENDING;
            tasks[seq].result[0] = '\0';
            seq++;
        }
    }

    // // Send initial token to the ring
    // RingMessage token = make_token_msg();
    // size_t bytes = write(ring_write_fd, &token, sizeof(RingMessage));
    // if (bytes == -1)
    // {
    //     perror("write token to ring");
    //     exit(EXIT_FAILURE);
    // }
    // printf("Parent sent initial token to ring.\n");

    // Inject all DATA messages into the ring
    for (int i = 0; i < total_tasks; i++)
    {
        RingMessage data = make_data_msg(tasks[i].receiver_id, tasks[i].task_id,
                                         tasks[i].sequence_num, tasks[i].filepath);
        size_t bytes = write(ring_write_fd, &data, sizeof(RingMessage));
        if (bytes == -1)
        {
            perror("write data to ring");
            exit(EXIT_FAILURE);
        }
        printf("Parent injected task %d (node %d, %s)\n",
               tasks[i].task_id, tasks[i].receiver_id, tasks[i].filepath);
    }

    // Collect results: each node sends exactly num_files reports
    int results_collected = 0;
    for (int n = 0; n < num_nodes; n++)
    {
        for (int r = 0; r < num_files; r++)
        {
            RingMessage result_msg;
            size_t rbytes = read(stat_read_fds[n], &result_msg, sizeof(RingMessage));
            if (rbytes == -1)
            {
                perror("read result from stat pipe");
                exit(EXIT_FAILURE);
            }
            if (rbytes == 0)
            {
                fprintf(stderr, "stat pipe closed unexpectedly for node %d\n", n);
                exit(EXIT_FAILURE);
            }
            if (result_msg.status == STATUS_ERROR)
            {
                printf("Parent received an error report for task %d from node %d: %s\n",
                       result_msg.task_id, result_msg.sender_id, result_msg.result);
                // Send shutdown message to the ring
                RingMessage shutdown = make_shutdown_msg();
                size_t bytes = write(ring_write_fd, &shutdown, sizeof(RingMessage));
                if (bytes == -1)
                {
                    perror("write shutdown to ring");
                    exit(EXIT_FAILURE);
                }
                printf("Parent sent shutdown to ring.\n");

                // Close parent's write end
                close(ring_write_fd);
                return; // Exit early on error report
            }

            // Match the report back to its task by sequence_num
            for (int t = 0; t < total_tasks; t++)
            {
                if (tasks[t].sequence_num == result_msg.sequence_num)
                {
                    tasks[t].status = result_msg.status;
                    strncpy(tasks[t].result, result_msg.result, MAX_PAYLOAD);
                    tasks[t].result[MAX_PAYLOAD - 1] = '\0';
                    results_collected++;
                    printf("Parent got result for task %d from node %d: %s\n",
                           tasks[t].task_id, result_msg.sender_id, tasks[t].result);
                    break;
                }
            }
        }
    }

    // Print the summary table: one row per task, one column per file
    printf("\n===== RESULTS SUMMARY =====\n");
    printf("%-22s", "Task");
    for (int f = 0; f < num_files; f++)
    {
        printf(" %-22.22s", file_paths[f]);
    }
    printf("\n");
    printf("%-22s", "----");
    for (int f = 0; f < num_files; f++)
    {
        printf(" %-22s", "----");
    }
    printf("\n");

    for (int n = 0; n < num_nodes; n++)
    {
        printf("%-22s", task_names[n]);
        for (int f = 0; f < num_files; f++)
        {
            // Tasks were injected in file-major order
            int idx = f * num_nodes + n;
            if (tasks[idx].status == STATUS_OK)
            {
                printf(" %-22.22s", tasks[idx].result);
            }
            else
            {
                printf(" %-22s", "FAILED");
            }
        }
        printf("\n");
    }

    // ACK/NACK accounting for the report
    printf("\nACK: %d/%d tasks completed successfully.\n",
           results_collected, total_tasks);

    // Send shutdown message to the ring
    RingMessage shutdown = make_shutdown_msg();
    size_t bytes = write(ring_write_fd, &shutdown, sizeof(RingMessage));
    if (bytes == -1)
    {
        perror("write shutdown to ring");
        exit(EXIT_FAILURE);
    }
    printf("Parent sent shutdown to ring.\n");

    // Close parent's write end
    close(ring_write_fd);
}
