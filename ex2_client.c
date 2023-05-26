#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

time_t startTime; // Get the current time
time_t currentTime;
time_t elapsedTime;
int resultFlag = 0;
char *resultFinal;

char *intToString(int number)// turns int to string without wasting any space
{
    int length = 1; // Initialize the length to 1 for the null terminator

    // Determine the length of the string representation
    int temp = number;
    while (temp /= 10)
    {
        length++;
    }

    // Allocate memory for the string representation
    char *result = (char *)malloc((length + 1) * sizeof(char));

    // Convert the integer to a string
    sprintf(result, "%d", number);

    return result;
}

char *readLine(int *fd)// reads a line from a fd 
{
    ssize_t bytes_read;
    char *line = (char *)malloc(sizeof(char));
    size_t idx = 0;
    char c;
    while ((bytes_read = read(*fd, &c, sizeof(char))) > 0 && c != '\n')
    {
        line = (char *)realloc(line, (idx + 1) * sizeof(char) + sizeof(char));
        line[idx++] = c;
    }

    line[idx] = '\0';
    return line;
}

int CheckForFile()//checks for the to server file, returns 1 if found
{
    const char *filename = "to_server.txt";
    DIR *dir;
    struct dirent *entry;

    dir = opendir(".");
    if (dir == NULL)
    {
        perror("opendir");
        return -1;
    }

    int fileFound = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, filename) == 0)
        {
            fileFound = 1;
            break;
        }
    }

    closedir(dir);

    return fileFound;
}

char *strCat(char *str1, char *str2)// strCat function
{
    char *concat = (char *)malloc(sizeof(char));
    size_t idx = 0;

    while (*str1 != '\0')
    {
        concat = (char *)realloc(concat, (idx + 1) * sizeof(char) + sizeof(char));
        concat[idx++] = *(str1++);
    }

    while (*str2 != '\0')
    {
        concat = (char *)realloc(concat, (idx + 1) * sizeof(char) + sizeof(char));
        concat[idx++] = *(str2++);
    }

    concat[idx] = '\0';
    return concat;
}

void handleServerSignal(int signal)// handles the servers signal 
{

    if (signal == SIGUSR2)
    {
        int cProcess = getpid();
        char *str1temp = intToString(cProcess);

        char *str1 = strCat("to_client_", str1temp);

        char *resulttxt = strCat(str1, ".txt");

        int resultFd = open(resulttxt, O_RDONLY);

        if (resultFd != -1)
        {
            resultFinal = readLine(&resultFd);
            resultFlag = 1;
            elapsedTime = 0;
            startTime = time(NULL); // restart start time
        }
        else
        {
            printf("Error reading server response\n");
        }

        close(resultFd);

        if (remove(resulttxt) == 0)
        {
            // File successfully removed
        }
        else
        {
            printf("Error removing server response file\n");
        }

        free(str1);
        free(resulttxt);
    }
}

int main(int argc, char *argv[])
{
    signal(SIGUSR2, handleServerSignal);

    if (argc != 5)
    {
        printf("ERROR FROM EX2 \n");
        return -1;
    }

    char *serverId = argv[1];
    char *num1 = argv[2];
    char *operator2 = argv[3];
    char *num2 = argv[4];

    // Initialize the random number generator
    srand(time(NULL));

    // Generate a random number between 1 and 5
    int randomNumber;

    int i;
    for (i = 0; i < 10; i++)
    {
        if (CheckForFile() == 0)
        {
            break;
        }
        else
        {
            randomNumber = rand() % 5 + 1;
            sleep(randomNumber);
        }
    }

    if (i == 10)
    {
        printf("file occupied");
        exit(0);
    }

    const char *filename = "to_server.txt";

    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    // Perform file operations here

    num1 = strCat(num1, "\n");

    operator2 = strCat(operator2, "\n");

    num2 = strCat(num2, "\n");

    int cProccess = getpid();
    char *str2 = intToString(cProccess);

    char *str1 = strCat(str2, "\n");

    int client = write(fd, str1, strlen(str1));
    int writenum1 = write(fd, num1, strlen(num1));
    int writeoperator = write(fd, operator2, strlen(operator2));
    int writenum2 = write(fd, num2, strlen(num2));

    sleep(8);

    if (kill(atoi(serverId), SIGUSR1) == 0)
    {
        printf("Signal sent successfully.\n");
    }
    else
    {
        printf("ERROR FROM EX2 \n");
        return -1;
    }

    close(fd);

    startTime = time(NULL); // Get the current time

    while (1)
    {
        currentTime = time(NULL); // Get the current time
        elapsedTime = currentTime - startTime;

        if (elapsedTime >= 30)
        {
            printf("Timer expired.\n");
            break;
        }
        if (resultFlag == 1)
        {
            printf("the server returned : %s \n", resultFinal);
            return 1;
        }
        time_t remainingTime = 30 - elapsedTime;
    }

    printf("Client closed because no response was received from the server for 30 seconds");

    return 1;
}
