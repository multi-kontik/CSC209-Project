#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "ring.h"

/* Simplifying assumptions:
 * 1. Every sentence starts on a new line (of the file).
 * 2. Every section ends on an empty line.
 */

// See https://stackoverflow.com/questions/13566082/how-to-check-if-a-file-has-content-or-not-using-c

char *word_count(FILE **file_ptr_ptr)
{
	int num_words = 0;
	FILE *file_ptr = *file_ptr_ptr;
	char word[MAX_PAYLOAD + 1];

	int c = fgetc(file_ptr);
	if (c == EOF)
	{
		// i.e. If the file is empty
		// Return 0 (zero words) as fallback
		char *empty_result = malloc(MAX_PAYLOAD * sizeof(char));

		if (empty_result == NULL)
		{
			fprintf(stderr, "malloc error");
			exit(1);
		}

		strncpy(empty_result, "0", MAX_PAYLOAD);
		empty_result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
		return empty_result;
	}
	else
	{
		ungetc(c, file_ptr);
	}

	while (!feof(file_ptr))
	{
		if (fscanf(file_ptr, "%255s", word) != 1)
		{
			// We know it is an error, since we already checked for EOF above
			fprintf(stderr, "fscanf error");
			return NULL;
		}
		else
		{
			num_words++;
		}
	}

	char *result = malloc(MAX_PAYLOAD * sizeof(char));
	if (result == NULL)
	{
		fprintf(stderr, "malloc error");
		exit(1);
	}

	snprintf(result, MAX_PAYLOAD, "%d", num_words);
	result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
	return result;
}

char *sentence_count(FILE **file_ptr_ptr)
{
	int num_sentences = 0;
	FILE *file_ptr = *file_ptr_ptr;
	char sentence[MAX_PAYLOAD + 1];

	int c = fgetc(file_ptr);
	if (c == EOF)
	{
		// i.e. If the file is empty
		// Return 0 (zero sentences) as fallback
		char *empty_result = malloc(MAX_PAYLOAD * sizeof(char));

		if (empty_result == NULL)
		{
			fprintf(stderr, "malloc error");
			exit(1);
		}

		strncpy(empty_result, "0", MAX_PAYLOAD);
		empty_result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
		return empty_result;
	}
	else
	{
		ungetc(c, file_ptr);
	}

	while (!feof(file_ptr))
	{
		if (fgets(sentence, MAX_PAYLOAD + 1, file_ptr) == NULL)
		{
			// We know it is an error, since we already checked for EOF above
			fprintf(stderr, "fgets error");
			return NULL;
		}
		else if (sentence[0] != '\n')
		{
			// Do *not* increment based on empty sentences
			num_sentences++;
		}
	}

	char *result = malloc(MAX_PAYLOAD * sizeof(char));
	if (result == NULL)
	{
		fprintf(stderr, "malloc error");
		exit(1);
	}

	snprintf(result, MAX_PAYLOAD, "%d", num_sentences);
	result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
	return result;
}

char *longest_sentence(FILE **file_ptr_ptr)
{
	char curr_sentence[MAX_PAYLOAD];
	char longest_sentence[MAX_PAYLOAD];
	int longest_sentence_length = 0;
	FILE *file_ptr = *file_ptr_ptr;

	int c = fgetc(file_ptr);
	if (c == EOF)
	{
		// i.e. If the file is empty
		// Return an empty string as fallback
		char *empty_result = malloc(MAX_PAYLOAD * sizeof(char));

		if (empty_result == NULL)
		{
			fprintf(stderr, "malloc error");
			exit(1);
		}

		empty_result[0] = '\0';
		return empty_result;
	}
	else
	{
		ungetc(c, file_ptr);
	}

	while (!feof(file_ptr))
	{
		if (fgets(curr_sentence, MAX_PAYLOAD + 1, file_ptr) == NULL)
		{
			// We know it is an error, since we already checked for EOF above
			fprintf(stderr, "fgets error");
			return NULL;
		}
		else
		{
			int curr_sentence_length = strlen(curr_sentence);

			// Update longest_sentence if necessary
			if (curr_sentence_length > longest_sentence_length)
			{
				strncpy(longest_sentence, curr_sentence, MAX_PAYLOAD);
				longest_sentence[MAX_PAYLOAD - 1] = '\0';
				longest_sentence_length = curr_sentence_length;
			}
		}
	}

	char *result = malloc(MAX_PAYLOAD * sizeof(char));
	if (result == NULL)
	{
		fprintf(stderr, "malloc error");
		exit(1);
	}

	strncpy(result, longest_sentence, MAX_PAYLOAD);
	result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
	return result;
}

char *longest_word(FILE **file_ptr_ptr)
{
	char curr_word[MAX_PAYLOAD];
	char longest_word[MAX_PAYLOAD];
	int longest_word_length = 0;
	FILE *file_ptr = *file_ptr_ptr;

	int c = fgetc(file_ptr);
	if (c == EOF)
	{
		// i.e. If the file is empty
		// Return an empty string as fallback
		char *empty_result = malloc(MAX_PAYLOAD * sizeof(char));

		if (empty_result == NULL)
		{
			fprintf(stderr, "malloc error");
			exit(1);
		}

		empty_result[0] = '\0';
		return empty_result;
	}
	else
	{
		ungetc(c, file_ptr);
	}

	while (!feof(file_ptr))
	{
		if (fscanf(file_ptr, "%255s", curr_word) != 1)
		{
			// We know it is an error, since we already checked for EOF above
			fprintf(stderr, "fscanf error");
			return NULL;
		}
		else
		{
			int curr_word_length = strlen(curr_word);

			// Update the longest word if necessary
			if (curr_word_length > longest_word_length)
			{
				strncpy(longest_word, curr_word, MAX_PAYLOAD);
				longest_word[MAX_PAYLOAD - 1] = '\0';
				longest_word_length = curr_word_length;
			}
		}
	}

	char *result = malloc(MAX_PAYLOAD * sizeof(char));
	if (result == NULL)
	{
		fprintf(stderr, "malloc error");
		exit(1);
	}

	strncpy(result, longest_word, MAX_PAYLOAD);
	result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
	return result;
}

int num_spaces_in_line(char line[])
{
	int space_count = 0;
	int line_length = strlen(line);

	for (int i = 0; i < line_length; i++)
	{
		if (line[i] == ' ')
		{
			space_count++;
		}
	}

	return space_count;
}

char *average_sentence_length(FILE **file_ptr_ptr)
{
	FILE *file_ptr = *file_ptr_ptr;
	int word_count = 0;
	int sentence_count = 0;
	char sentence[MAX_PAYLOAD];

	int c = fgetc(file_ptr);
	if (c == EOF)
	{
		// i.e. If the file is empty
		// Return 0.0 as fallback
		char *empty_result = malloc(MAX_PAYLOAD * sizeof(char));

		if (empty_result == NULL)
		{
			fprintf(stderr, "malloc error");
			exit(1);
		}

		strncpy(empty_result, "0.0", MAX_PAYLOAD);
		empty_result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
		return empty_result;
	}
	else
	{
		ungetc(c, file_ptr);
	}

	while (!feof(file_ptr))
	{
		if (fgets(sentence, MAX_PAYLOAD + 1, file_ptr) == NULL)
		{
			// We know it is an error, since we already checked for EOF above
			fprintf(stderr, "fgets error");
			return NULL;
		}
		else if (sentence[0] != '\n')
		{
			// Sentence non-empty
			word_count = word_count + (num_spaces_in_line(sentence) + 1);
			sentence_count++;
		}
	}

	char *result = malloc(MAX_PAYLOAD * sizeof(char));
	if (result == NULL)
	{
		fprintf(stderr, "malloc error");
		exit(1);
	}

	snprintf(result, MAX_PAYLOAD, "%f", ((float)word_count / sentence_count));
	result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
	return result;
}

int char_count_in_line(char line[])
{
	int char_count = 0;
	int line_length = strlen(line);

	for (int i = 0; i < line_length; i++)
	{
		if (line[i] != ' ')
		{
			char_count++;
		}
	}

	return char_count;
}

char *average_word_length(FILE **file_ptr_ptr)
{
	FILE *file_ptr = *file_ptr_ptr;

	// The total number of characters, excluding spaces,
	//  taken up by every word in this file
	int total_word_chars = 0;

	int word_count = 0;
	char sentence[MAX_PAYLOAD];

	int c = fgetc(file_ptr);
	if (c == EOF)
	{
		// i.e. If the file is empty
		// Return 0.0 as fallback
		char *empty_result = malloc(MAX_PAYLOAD * sizeof(char));

		if (empty_result == NULL)
		{
			fprintf(stderr, "malloc error");
			exit(1);
		}

		strncpy(empty_result, "0.0", MAX_PAYLOAD);
		empty_result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
		return empty_result;
	}
	else
	{
		ungetc(c, file_ptr);
	}

	while (!feof(file_ptr))
	{
		if (fgets(sentence, MAX_PAYLOAD + 1, file_ptr) == NULL)
		{
			// We know it is an error, since we already checked for EOF above
			fprintf(stderr, "fgets error");
			return NULL;
		}
		else
		{
			word_count = word_count + (num_spaces_in_line(sentence) + 1);
			total_word_chars += char_count_in_line(sentence);
		}
	}

	char *result = malloc(MAX_PAYLOAD * sizeof(char));
	if (result == NULL)
	{
		fprintf(stderr, "malloc error");
		exit(1);
	}

	snprintf(result, MAX_PAYLOAD, "%f", ((float)total_word_chars / word_count));
	result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
	return result;
}

char *section_count(FILE **file_ptr_ptr)
{
	FILE *file_ptr = *file_ptr_ptr;

	int c = fgetc(file_ptr);
	if (c == EOF)
	{
		// i.e. If the file is empty
		// Return 0 (zero sections) as fallback
		char *empty_result = malloc(MAX_PAYLOAD * sizeof(char));

		if (empty_result == NULL)
		{
			fprintf(stderr, "malloc error");
			exit(1);
		}

		strncpy(empty_result, "0", MAX_PAYLOAD);
		empty_result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
		return empty_result;
	}
	else
	{
		ungetc(c, file_ptr);
	}

	// We start at 1 because if there are no blank lines,
	//  then the entire document will constitute one section (and so on)
	int num_sections = 1;

	char sentence[MAX_PAYLOAD + 1];

	while (!feof(file_ptr))
	{
		if (fgets(sentence, MAX_PAYLOAD + 1, file_ptr) == NULL)
		{
			// We know it is an error, since we already checked for EOF above
			fprintf(stderr, "fgets error");
			return NULL;
		}
		else if (sentence[0] == '\n')
		{
			// Empty line, increment section count by one
			num_sections++;
		}
	}

	char *result = malloc(MAX_PAYLOAD * sizeof(char));
	if (result == NULL)
	{
		fprintf(stderr, "malloc error");
		exit(1);
	}

	snprintf(result, MAX_PAYLOAD, "%d", num_sections);
	result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
	return result;
}

char *character_count(FILE **file_ptr_ptr)
{
	int total_length = 0;
	FILE *file_ptr = *file_ptr_ptr;
	char sentence[MAX_PAYLOAD + 1];

	int c = fgetc(file_ptr);
	if (c == EOF)
	{
		// i.e. If the file is empty
		// Return 0 (zero characters) as fallback
		char *empty_result = malloc(MAX_PAYLOAD * sizeof(char));

		if (empty_result == NULL)
		{
			fprintf(stderr, "malloc error");
			exit(1);
		}

		strncpy(empty_result, "0", MAX_PAYLOAD);
		empty_result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
		return empty_result;
	}
	else
	{
		ungetc(c, file_ptr);
	}

	while (!feof(file_ptr))
	{
		if (fgets(sentence, MAX_PAYLOAD + 1, file_ptr) == NULL)
		{
			// We know it is an error, since we already checked for EOF above
			fprintf(stderr, "fgets error");
			return NULL;
		}
		else
		{
			// Subtract one per line for the trailing new-line character
			// Since the last line may not have such a character, we add one back below
			total_length = total_length + strlen(sentence) - 1;
		}
	}

	total_length++;

	char *result = malloc(MAX_PAYLOAD * sizeof(char));
	if (result == NULL)
	{
		fprintf(stderr, "malloc error");
		exit(1);
	}

	snprintf(result, MAX_PAYLOAD, "%d", total_length);
	result[MAX_PAYLOAD - 1] = '\0'; // Null-termination
	return result;
}

/*
 * Find the correct task function, all of which takes in FILE * and outputs char[].
 * @node_id: The ID of this node, used to find the correct task function.
 */

char *(*find_task(int node_id))(FILE **)
{
	char *(*task_func)(FILE **);

	// Find task function
	// See https://www.w3schools.com/c/c_switch.php
	switch (node_id)
	{
	case TASK_WORD_COUNT:
		task_func = word_count;
		break;
	case TASK_AVERAGE_SENTENCE_LENGTH:
		task_func = average_sentence_length;
		break;
	case TASK_LONGEST_SENTENCE_LENGTH:
		task_func = longest_sentence;
		break;
	case TASK_LONGEST_WORD:
		task_func = longest_word;
		break;
	case TASK_AVERAGE_WORD_LENGTH:
		task_func = average_word_length;
		break;
	case TASK_SENTENCE_COUNT:
		task_func = sentence_count;
		break;
	case TASK_SECTION_COUNT:
		task_func = section_count;
		break;
	case TASK_CHARACTER_COUNT:
		task_func = character_count;
		break;
	default:
		fprintf(stderr, "Invalid node_id");
		exit(1);
	}

	return task_func;
}

char *task(RingMessage rm)
{
	FILE *file_ptr = fopen(rm.payload, "r");
	if (file_ptr == NULL)
	{
		fprintf(stderr, "fopen error: could not open file or file does not exist: %s\n", rm.payload);
		exit(1);
	}
	else
	{
		// Find task
		char *(*task)(FILE **) = find_task(rm.receiver_id);

		// Do task and store results in ret; close file
		char *ret = task(&file_ptr);
		if (fclose(file_ptr) == -1)
		{
			fprintf(stderr, "fclose error on file %s\n", rm.payload);
			return NULL;
		}
		return ret;
	}
}
