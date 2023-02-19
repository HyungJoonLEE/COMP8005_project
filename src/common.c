#include "common.h"
#include "error.h"
#include "server.h"
#include "linkedList.h"
#include <assert.h>


int read_from_shadow(char *file_directory, char *file_list) {
    char buffer[BUF_SIZE];
    char sudo_string[70] = {0};
    FILE *fp;

    sprintf(sudo_string, "sudo -S cat %s", file_directory);
    memset(buffer, 0, BUF_SIZE);
    fp = popen(sudo_string, "r");
    if (fp == NULL) {
        perror("popen() failed\n");
        return -1;
    }

    while (fgets(buffer, BUF_SIZE, fp)) {
        strcat(file_list, buffer);
    }
    pclose(fp);
    return 0;
}


void find_user(char *file_list, LinkedList *user_list) {
    char temp[BUF_SIZE] = {0};
    strcpy(temp, file_list);
    memset(file_list, 0, BUF_SIZE);

    for (int i = 0; i < user_list->currentElementCount; i++) {
        strcpy(file_list, temp);
        char *line = strtok(file_list, "\n");
        while (line != NULL) {
        if (strncmp(line, getLLElement(user_list, i)->id,
                  strlen(getLLElement(user_list, i)->id)) == 0) {
            printf("%s\n", line);
            save_user(line, user_list, i);
            break;
        }
        line = strtok(NULL, "\n");
        }
    }
}


void save_user(char *user_info, LinkedList *user_list, int i) {
    char temp[256] = {0};
    char without_date[256] = {0};
    char *user_info_var = NULL;

    strcpy(temp, user_info);
    user_info += strlen(getLLElement(user_list, i)->id) + 1;
    strcpy(without_date, user_info);
    strtok(without_date, ":");
    strcpy(getLLElement(user_list, i)->original, without_date);

    user_info_var = strtok(user_info, "$");
    if (strcmp(user_info_var, "y") == 0) {
        strcpy(getLLElement(user_list, i)->hash_id, "y");
        strcpy(getLLElement(user_list, i)->hash_type, "yescrypt");
        user_info_var = strtok(NULL, "$");
        strcpy(getLLElement(user_list, i)->crypt_parm, user_info_var);
    }

    if (strcmp(user_info_var, "1") == 0) {
        strcpy(getLLElement(user_list, i)->hash_id, "1");
        strcpy(getLLElement(user_list, i)->hash_type, "MD5");
    }
    if (strcmp(user_info_var, "5") == 0) {
        strcpy(getLLElement(user_list, i)->hash_id, "5");
        strcpy(getLLElement(user_list, i)->hash_type, "SHA-256");
    }
    if (strcmp(user_info_var, "6") == 0) {
        strcpy(getLLElement(user_list, i)->hash_id, "6");
        strcpy(getLLElement(user_list, i)->hash_type, "SHA-512");
    }
    save_userinfo(temp, user_list, i);
}


void save_userinfo(char *user_info, LinkedList *user_list, int i) {
    char *user_info_var;
    user_info += strlen(getLLElement(user_list, i)->id) +
                    strlen(getLLElement(user_list, i)->hash_id);

    if (strcmp(getLLElement(user_list, i)->hash_id, "y") == 0)
        user_info += strlen(getLLElement(user_list, i)->crypt_parm) + 4;
    else
        user_info += 3;

    user_info_var = strtok(user_info, "$");
    strcpy(getLLElement(user_list, i)->salt, user_info_var);
    user_info_var = strtok(NULL, ":");
    strcpy(getLLElement(user_list, i)->hash_value, user_info_var);
}


void salt_setting(LinkedList *user_list) {
    char salt_setting[200] = {0};
    for (int i = 0; i < user_list->currentElementCount; i++) {
        if (strcmp(getLLElement(user_list, i)->hash_id, "y") == 0)
            sprintf(salt_setting, "$%s$%s$%s", getLLElement(user_list, i)->hash_id,
                getLLElement(user_list, i)->crypt_parm,
                getLLElement(user_list, i)->salt);
        else
            sprintf(salt_setting, "$%s$%s", getLLElement(user_list, i)->hash_id,
                getLLElement(user_list, i)->salt);

        strcpy(getLLElement(user_list, i)->salt_setting, salt_setting);
    }
}


void password_generator(int *ptr1, int *ptr2, int temp_pwlen, LinkedList *user_list, int user_index) {
    char password[temp_pwlen];
    int temp[temp_pwlen + 1];
    int i, j;

    memset(temp, 0, temp_pwlen);
    memset(password, 0, temp_pwlen);

    for (i = 0; i < temp_pwlen; i++)
        temp[i] = ptr1[i];

    i = 0;
    while (i < temp_pwlen) {
        for (i = 0; i < temp_pwlen; i++) {
            password[i] = passwd_arr[temp[i]];
        }
        password[temp_pwlen] = 0;
        printf("[thread %d]: %s\n", omp_get_thread_num(), password);
#pragma omp critical
        if (strcmp(
            crypt(password, getLLElement(user_list, user_index)->salt_setting),
            getLLElement(user_list, user_index)->original) == 0) {
                strcpy(getLLElement(user_list, user_index)->password, password);
        }

        if (strlen(getLLElement(user_list, user_index)->password) > 0) {
            return;
        }

        getLLElement(user_list, user_index)->count++;

        for (i = 0; i < temp_pwlen && temp[temp_pwlen - i - 1]++ == PASS_ARR_LEN; i++)
            temp[temp_pwlen - i - 1] = 0;

        for (j = 0; j < temp_pwlen; j++)
            if (temp[j] != ptr2[j])
                break;
        if (j == temp_pwlen)
        return;
    }
}


void free_heap_memory(LinkedList *user_list) {
  for (int i = 0; i < getLinkedListLength(user_list); i++) {
    removeLLElement(user_list, i);
  }
}
