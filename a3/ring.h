#ifndef RING_H
#define RING_H

#define MAX_NODES 6
#define MAX_PAYLOAD 256
#define MAX_FACTORS 64

typedef enum
{
    MSG_TOKEN,   // 0 - free token, no data
    MSG_DATA,    // 1 - task travelling to a specific node
    MSG_RESULT,  // 2 - node sending result to parent
    MSG_ACK,     // 3 - task completed successfully
    MSG_NACK,    // 4 - task failed or not claimed
    MSG_SHUTDOWN // 5 - terminate gracefully
} MessageType;

typedef enum
{
    STATUS_PENDING, // 0 - task is pending, waiting to be processed
    STATUS_OK,      // 1 - task completed successfully, result is valid
    STATUS_ERROR    // 2 - task failed, result is invalid
} TaskStatus;

typedef enum
{
    TASK_WORD_COUNT,
    TASK_LONGEST_SENTENCE_LENGTH,
    TASK_AVERAGE_SENTENCE_LENGTH,
    TASK_LONGEST_WORD,
    TASK_AVERAGE_WORD_LENGTH, 
    TASK_SENTENCE_COUNT,
    TASK_SECTION_COUNT,
    TASK_CHARACTER_COUNT
} TaskType;

typedef struct
{
    MessageType type;          // what kind of message is this?
    int sender_id;             // who created/sent this message?
    int receiver_id;           // which node should process this? (-1 = everyone)
    int task_id;               // unique task identifier
    int sequence_num;          // order this task was sent by parent
    int hop_count;             // how many nodes has this passed through?
    TaskStatus status;         // pending / completed / error
    char payload[MAX_PAYLOAD]; // input data travelling TO the worker
    char result[MAX_PAYLOAD];  // output data travelling back TO the parent
} RingMessage;

void run_node(int node_id, int ring_read_fd, int ring_write_fd, int stat_write_fd);

RingMessage make_report_msg(int node_id, TaskStatus status, const char *result);
RingMessage make_token_msg();
RingMessage make_shutdown_msg();

char* task(RingMessage rm);

#endif
