#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <stdio.h>

typedef struct s_data
{
    char *args[1000];
    int pipe[2]; // если 2, дальше pipe
    int next;
} t_data;

int ft_strlen(char *str)
{
    int i = 0;
    while(str[i])
        i++;
    return(i);
}

void ft_error(int fd, char *str1, char *str2)
{
    write(fd, str1, ft_strlen(str1));
    if (str2 != NULL)
        write(fd, str2, ft_strlen(str2));
    write(fd, "\n", 1);
}

void parse(t_data *data, char **argv)
{
    int i = 1;
    int j = 0;
    int cmd = 0;

    while(argv[i] != NULL)
    {
        if (strcmp(argv[i], "|") == 0)
        {
            data[cmd].next = 2;
            j = 0;
            cmd++;
        }
        else if (strcmp(argv[i], ";") == 0)
        {
            data[cmd].next = 1;
            j = 0;
            cmd++;
        }
        else
        {
            data[cmd].next = 1;
            data[cmd].args[j] = argv[i];
            j++;
        }
        i++;
    }
}

void run(t_data *data, char **env)
{
    int res = 0;
    int i = 0;

    while(data[i].next != 0)
    {
        if (data[i].args[0] != NULL && strcmp(data[i].args[0], "cd") == 0)
        {
            if (data[i].args[2] != NULL || data[i].args[1] == NULL || data[i].args[1][0] == '-')
                ft_error(2, "bad arguments", NULL);
            else if (chdir(data[i].args[1]) < 0)
                ft_error(2, "cannot change to directory to", data[i].args[1]);
        }
        else
        {
            if (data[i].next == 2)
                pipe(data[i].pipe);
            res = fork();
            if (res == 0)
            {
                if (i != 0 && data[i - 1].next == 2)
                    dup2(data[i - 1].pipe[0], 0);
                if (data[i].pipe[1] != 0)
                    dup2(data[i - 1].pipe[0], 1);
                if (data[i].args[0] != 0 && execve(data[i].args[0], data[i].args, env) < 0)
                    ft_error(2, "cannot execve", data[i].args[0]);
                exit(1);
            }
            else
            {
                waitpid(res, NULL, 0);
                if (i != 0 && data[i - 1].next == 2)
                    close(data[i - 1].pipe[0]);
                if (data[i].pipe[1] != 0)
                    close(data[i].pipe[1]);
            }
        }
        i++;
    }
}

// void run(t_data *data, char **env)
// {
//     int res = 0;
//     int i = 0;

//     while(data[i].next != 0)
//     {
//         if (data[i].args[0] != NULL && strcmp(data[i].args[0], "cd") == 0) //если первый аргумент cd и он не равен нулю
//         {
//             // не обрабавываем cd -
//             if (data[i].args[2] != NULL || data[i].args[1] == NULL || data[i].args[1][0] == '-') //! последнее условие должно оставаться последним
//                 ft_error(2, "error: cd: bad arguments", NULL);
//             else if (chdir(data[i].args[1]) < 0)
//                 ft_error(2, "error: cd: cannot change directory to", data[i].args[1]);
//         }
//         else //* если команда не cd
//         {
//             if (data[i].next == 2)
//                 pipe(data[i].pipe);
//             res = fork(); //форкаем
//             if (res == 0)
//             {
//                 if (i != 0 && data[i - 1].next == 2) //след команда после пайпа
//                     dup2(data[i - 1].pipe[0], 0);  //то для пред команды переопределяем fd на пайп 0
//                 if (data[i].pipe[1] != 0)
//                     dup2(data[i - 1].pipe[0], 1);
//                 if (data[i].args[0] != 0 && execve(data[i].args[0], data[i].args, env) < 0)
//                     ft_error(2, "error: cannot execute ", data[i].args[0]);
//                 exit(1);
//             }
//             	else //* родительский процесс
// 			{
// 				waitpid(res, NULL, 0); // ждет завершения дочернего процесса, чтобы закрыть fd-шки
// 				if (i != 0 && data[i - 1].next == 2)
// 					close(data[i - 1].pipe[0]); // закрывает fd команды которая перед пайпом
// 				if (data[i].pipe[1] != 0)
// 					close(data[i].pipe[1]); // закрывает fd команды которая после пайпа
// 			}
// 		}
// 		i++;
//     }
// }


int main(int argc, char **argv, char **env)
{
    t_data data[1000];

    if (argc > 1)
    {
        parse(data, argv);
        run(data, env);
    }
}