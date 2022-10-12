#include <unistd.h> // pipe, dup, fork, write, chdir, execve, close,
#include <string.h> // strcmp
#include <stdlib.h> // exit, malloc, waitpid

#include <stdio.h> //не нужен, только для printf

typedef struct s_data
{
	char *args[1000];
	int pipe[2];
	int next; //* если 2 то после команды есть пайп, если 1 то нет пайпа
}t_data;

int ft_strlen (char *str)
{
	int i = 0;
	while(str[i])
		i++;
	return (i);
}
void ft_error (int fd, char *str1, char *str2)
{
	write(fd, str1, ft_strlen(str1));
	if (str2 != NULL)
		write (fd, str2, ft_strlen(str2));
	write (fd, "\n", 1);
}

void parse (t_data *data, char **argv)
{
	int i = 1;
	int cmd = 0;
	int j = 0;

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

	// printf("data[%d].next= %d\n", i, data[i].next); //! убрать

	while(data[i].next != 0) //*
	{
		if (data[i].args[0] != NULL && strcmp(data[i].args[0], "cd") == 0) //! 1-е усл-е обязательно
		{ //* команда cd выполняется с одним аргументом, но не обрабатываем cd -
			if (data[i].args[2] != NULL || data[i].args[1] == NULL || data[i].args[1][0] == '-') //! последнее условие должно оставаться последним
				ft_error(2, "error: cd: bad arguments", NULL);
			else if (chdir(data[i].args[1]) < 0) //* выполнение cd и сразу проверка если не удалось то ошибка
				ft_error(2, "error: cd: cannot change directory to ", data[i].args[1]);
		}
		else //* если команда не cd
		{
			if (data[i].next == 2) //* означает что после команды идет пайп
				pipe(data[i].pipe);
			res = fork();
			if (res == 0) //* дочерний процесс
			{
				if (i != 0 && data[i - 1].next == 2)//* если это след ком. после пайпа
					dup2(data[i - 1].pipe[0], 0); //то для пред команды переопределяем fd на пайп 0
				if (data[i].pipe[1] != 0) //? если для текущей команды пайп был создан т.е. перед ней был пайп
					dup2(data[i].pipe[1], 1); // то установливаем его в 1
				if (data[i].args[0] != 0 && execve(data[i].args[0], data[i].args, env) < 0) //* если есть команда, то выполняем ее и проверяем выполнилась ли она
					ft_error(2, "error: cannot execute ", data[i].args[0]);
				exit (1); //* выходим из дочернего процесса если была ошибка, если ошибок не было то после execve программа сама выйдет из доч/процесса
			}
			else //* родительский процесс
			{
				waitpid(res, NULL, 0); // ждет завершения дочернего процесса, чтобы закрыть fd-шки
				if (i != 0 && data[i - 1].next == 2)
					close(data[i - 1].pipe[0]); // закрывает fd команды которая перед пайпом
				if (data[i].pipe[1] != 0)
					close(data[i].pipe[1]); // закрывает fd команды которая после пайпа
			}
		}
		i++;
		// printf("data[%d].next= %d\n", i, data[i].next); //! убрать
	}
}

//* это вариант без маллока, на экзамене проверен
int main (int argc, char **argv, char **env)
{
	t_data	data[1000];

	if (argc > 1)
	{
		parse(data, argv);
		run(data, env);
	}
	return (0);
}

//* это вариант с маллоком, на экзамене проверен
// int main (int argc, char **argv, char **env)
// {
// 	t_data	*data;

// 	data = malloc(sizeof(t_data) * 1000);
// 	if (data == NULL)
// 	{
// 		ft_error(2, "error: fatal", NULL);
// 		exit (1);
// 	}
// 	if (argc > 1)
// 	{
// 		parse(data, argv);
// 		run(data, env);
// 	}
// 	return (0);
// }