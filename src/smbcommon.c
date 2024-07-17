//
// Created by maximizzar on 17.07.24.
//

#include "smbcommon.h"

/*
 * Serializes a Message into a buffer
 * - Makes sure the buffer is Zeroed
 * - Checks if the necessary fields are filled
 */
int socket_serialization(char buffer[MAX_BUFFER_SIZE], Message *message) {
    memset(buffer, 0, MAX_BUFFER_SIZE);

    if (!message->unix_timestamp) {
        fprintf(stderr, "Serialization Failed: No UNIX Timestamp provided!");
        return EXIT_FAILURE;
    }

    if (strcmp(message->topic, "") == 0) {
        fprintf(stderr, "Serialization Failed: No Topic provided!");
        return EXIT_FAILURE;
    }

    if (strcmp(message->data, "") == 0) {
        sprintf(buffer, "\"%ld\" \"%s\"",
                message->unix_timestamp, message->topic);
        return EXIT_SUCCESS;
    }

    sprintf(buffer, "\"%ld\" \"%s\" \"%s\"",
            message->unix_timestamp, message->topic, message->data);
    return EXIT_SUCCESS;
}

/*
 * De-Serializes a Message from a buffer
 * - Makes sure the Message struct is Zeroed
 * - Validates value count in the buffer
 * - Safes buffer in dyn char* array values
 * - Validates each value -> copy's it into Message
 *
 */
int socket_deserialization(char buffer[MAX_BUFFER_SIZE], Message *message) {
    memset(message, 0, sizeof(Message));
    int count = 0;

    // Count the number of values in the buffer
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] == '"') {
            count++;
        }
    }

    /* Check if a correct number of values was found. */
    if (count == 0) {
        fprintf(stderr, "Serialization Failed: No values in buffer!");
        return EXIT_FAILURE;
    }

    if (count / 2 > 3) {
        fprintf(stderr, "Serialization Failed: Too Many values in buffer!");
        return EXIT_FAILURE;
    }

    /* If Parsing makes likely sense, allocate memory */
    char** values = (char**)malloc(count * sizeof(char*));

    // Parse the values based on the count
    char* token = strtok(buffer, "\"");
    int index = 0;
    while (token != NULL) {
        if (strcmp(token, " ") != 0 && strcmp(token, "") != 0) {
            values[index] = strdup(token);
            index++;
        }
        token = strtok(NULL, "\"");
    }
    memset(buffer, 0, MAX_BUFFER_SIZE);

    message->unix_timestamp = (time_t) strtol(values[0], NULL, 0);
    if (!message->unix_timestamp) {
        fprintf(stderr, "Serialization Failed: Couldn't get UNIX Timestamp");
        // Free allocated memory
        for (int i = 0; i < count; i++) {
            free(values[i]);
            values[i] = NULL;
        }
        free(values);
        return EXIT_FAILURE;
    }

    strcpy(message->topic, values[1]);

    if (count == 3) {
        strcpy(message->data, values[2]);
    }

    // Free allocated memory
    for (int i = 0; i < count; i++) {
        free(values[i]);
        //values[i] = NULL;
    }
    free(values);
    return EXIT_SUCCESS;
}

char* timestamp_to_string(time_t timestamp) {
    struct tm *local_time = localtime(&timestamp);
    return asctime(local_time);
}
