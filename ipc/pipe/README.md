# System Programming - Process and Threads, Interprocess Communication 

這篇文章是我在 ptt 上看到的，是介紹 System Programming 中 `fork`, `exec*`, `pipe`, `dup2` 的概念。 原文章放在 ptt 上，但我有時候會遇上 ptt 的文章幾年後就無法看到的情況，只是想放上 GitHub 做為備份，因此此文的所有權皆為原作者 LoganChien (簡子翔)所有。我也非常感謝原文作者非常的解釋。

原文出處：
- [[系程] 教學: 簡介 fork, exec*, pipe, dup2](https://www.ptt.cc/bbs/b97902HW/M.1268932130.A.0CF.html)
- [Re: [系程] 教學: 簡介 fork, exec*, pipe, dup2](https://www.ptt.cc/bbs/b97902HW/M.1268953601.A.BA9.html)

以下為原文

# 簡介 fork, exec*, pipe, dup2

## 前言

鄭卜壬老師在上課的時候，以極快的速度講過了 `fork`, `exec*`, `pipe`, `dup2` 幾個指令，並以下面的程式示範了一個 Shell 如何實作 Redirection。我自己覺得這一部分很有趣，所以花了一些時間去做一些實驗，以下就把我的心得和大家分享。

## Redirection

我們先從老師課堂上的那一個 Redirection 程式談起。

```c
/* 程式碼 red.c */

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    /* -- Check Arguments -------- */
    if (argc < 4)
    {
        fprintf(stderr, "Error: Incorrect Arguments.\n");
        fprintf(stderr, "Usage: red STDIN_FILE STDOUT_FILE EXECUTABLE ARGS\n");
        exit(EXIT_FAILURE);
    }


    /* -- Open the Files for Redirection -------- */

    int fd_in = open(argv[1], O_RDONLY);

    if (fd_in < 0)
    {
        fprintf(stderr, "Error: Unable to open the input file.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd_out < 0)
    {
        fprintf(stderr, "Error: Unable to open the output file.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }


    /* -- Replace the Executable Image of the Process -------- */
    if (execvp(argv[3], argv + 3) == -1)
    {fprintf(stderr, "Error: Unable to load executable image.\n");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
```

大家可以用 `gcc red.c -o red` 來編譯這一個程式，然後用

```
./red 標準輸入 標準輸出 可執行檔 其他參數
```

來執行他。例如：

```
touch empty.txt
./red empty.txt output.txt /bin/echo "Hello, redirection."
```

我們可以發現 `/bin/echo` 執行時，其標準輸出被我們重新導向到 `output.txt`，而不是直接顯示到螢幕上。接下來我們來細看 red 這一個程式。我們先去除所有的錯誤檢查：

```c
int main(int argc, char **argv)
{
    int fd_in = open(argv[1], O_RDONLY);
    /* 很明顯地，這一行是要開啟一個檔案做為標準輸入的來源。

       fd_in = 3

       fd[0] --------> file_table[i] (inherit from shell)
       fd[1] --------> file_table[j] (inherit from shell)
       fd[2] --------> file_table[k] (inherit from shell)
       fd[3] --------> file_table[m] (open argv[1])
    */

    dup2(fd_in, STDIN_FILENO);
    /* 接著我們會先關掉原本由 bash 之類的 Shell 自動幫我們開啟的 STDIN。
       這裡的 STDIN_FILENO 事實上就是被 define 成 0。

       然後，把第 fd_in 個 File descriptor 複製一份到第 0 個 File descriptor。
       這時，同時會有二個 File descriptor 指向同一個檔案（Kernel File Table
       同一個 entry）。

       fd[0] ----------------------------+
       fd[1] --------> file_table[j]     |
       fd[2] --------> file_table[k]     |
       fd[3] --------> file_table[m] <---+
    */

    close(fd_in);
    /* 關閉第 fd_in 個 File descriptor。或者精確地說，回收第 fd_in 個
       File descriptor。因為我們不需要這個 File descriptor 了。此時，
       就只剩一個 File descriptor 指向 file_table[m]。

       fd[0] --------> file_table[m] (open argv[1])
       fd[1] --------> file_table[j] (inherit from shell)
       fd[2] --------> file_table[k]
    */


    int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    /* 開啟一個檔案準備做為標準輸出。如果檔案不存在，就自動建立他，並將
       其屬性設為 644。若檔案已經存在，記得要清空整個檔案。

       fd_out = 3

       fd[0] --------> file_table[m] (open argv[1])
       fd[1] --------> file_table[j] (inherit from shell)
       fd[2] --------> file_table[k]
       fd[3] --------> file_table[n] (open argv[2])
    */

    dup2(fd_out, STDOUT_FILENO);
    /* 和 STDIN 一樣，我們關閉原有的 STDOUT，把我們開啟的 fd_out 複製過去。
       fd[0] --------> file_table[m] (open argv[1])
       fd[1] --------------------------------+
       fd[2] --------> file_table[k]         |
       fd[3] --------> file_table[n] <-------+
    */

    close(fd_out);
    /* 和 STDIN 一樣，把 fd_out 回收掉。
       fd[0] --------> file_table[m] (open argv[1])
       fd[1] --------> file_table[n] (open argv[2])
       fd[2] --------> file_table[k] (inherit from shell)
    */

    execvp(argv[3], argv + 3);
    /* 重新載入另一個執行檔。我們下面會再談到他。所有的 File descriptor 都
       不會被改動 (註 1)。而 argv[3] 是可執行檔的位置，argv + 3 是因為除了前
       三個 argument 之外，我要把剩下的 argument 傳給另一個執行檔的 main() */
}
```

> 註 1: 對於沒有設定 FD_CLOEXEC 的 File descriptor，File descriptor 就不會因為 exec* 函式而被更動。本例之中，所有的 File descriptor 都沒有設 FD_CLOEXEC。

## exec* 重新載入可執行檔

所謂的可執行檔 (Executable)，就是我們在 Windows 上常見的 `.exe`，或者是 Linux 上的 elf executable。這種檔案會儲存許多 instruction 的 machine code。 **而 `exec*` 系統呼叫就是幫我們把可執行檔的 machine code 搬進 Process 的 Memory，然後呼叫可執行檔之中的 `main` 函式。** 所以 `exec*` 系統呼叫的第一個參數通常都是可執行檔的位置。

另外，直得注意的是： **如果 `exec*` 系統呼叫成功地被執行，這個 process 就好像一個人完全失憶，然後大腦被載入不同的記憶，他會完全忘記他本來要執行的指令，從新的 main 函式開始。不過應該屬於他的東西還會是他的，例如：Process ID、File descriptor 等等。** 

想像一下，如果今天 LxxxxCxxxx 失憶了，然後有一個很強的催眠師透過一些暗示，讓 LxxxxCxxxx 有外星人的記憶。於是我就完全忘了我是地球人，也忘了我寫過這一篇教學文，也忘了我等一下要去睡覺。我說起話來像是外星人，我的動作看起來像是外星人，事實上，我就是外星人。不過我身邊的東西，例如身分證上的身分證字號還是一樣，我手邊的書還是 Alho 的 Compiler，不會因為很強的催眠師而改變。

如果今天，有一個 process 本來他的記憶是 `a.out`，不過不久之後被 `exec*` 系統呼叫載入 `/bin/ls`。於是這個 process 完全忘了他是 `a.out`，忘了他曾經呼叫過 `exec*`。他的輸出結果像是 `/bin/ls`，他的執行步驟看起來像是 `/bin/ls`，事實上他就是 `/bin/ls`。不過這個 process 有的東西：Process ID、File Descriptor 不會因為exec 而改變。

所以上面的 `red.c` 在 `exec*` 載入程式之後，如果直接把 `fd[1]` 當作 `stdout`，則所有的標準輸出就會被寫到我們開啟的檔案。

XD

還有，眼尖的同學可能已經注意到了！我上面所有的 `exec` 都加上了 *，而在我的程式碼之中，我呼叫的是 `execv` 的函式。事實上，為了方便 programmer 使用，`exec*` 有很多變體，如 `execl`, `execv`, `execle`, `execve`, `execlp`, `execvp`, `execlp`。他們的差異是：結尾有 l 的，就是函式長度不固定 (va_arg) 的版本。這方便我們在程式之中直接呼叫 `exec`，例如：

```c
execl("/bin/echo", "echo", "abc", "def", "ghi", (char *)0);
```

注意，一定要有一個 0 做為 argument 的結尾。如果是用以上的系統呼叫，`argv[0]` 會是 "echo"，`argv[1]` 會是 "abc"，等等，而 `argc` 會是 4。

結尾有 v 的，就是 `char **` 的版本。一樣地，`char **` 的最後一個參數要以 `0` 做為結尾。

```c
char *argvs[] = { "echo", "abc", "def", "ghi", 0 };
execv("/bin/echo", argvs);
```

結尾有 e 的就是可以設定 environment variable 的版本。例如：

```c
extern char **environ;
int main()
{
  execle("/bin/echo", "echo", "abc", "def", "ghi", (char *)0, environ);
}
```

結尾有 p 的就是會自動去從 `$PATH` 找出 executable 的版本。

```c
       /*file*/
execlp("echo", "echo", "abc", "def", "ghi", (char *)0);
execlp("ls", "ls", "-al", (char *)0);
```

ref. [http://pubs.opengroup.org/onlinepubs/000095399/functions/exec.html](http://pubs.opengroup.org/onlinepubs/000095399/functions/exec.html)

## fork 建立一個 Child Process

看完上面的 exec 你可能會想：不對呀，我不過是想要呼叫其他的程式，怎麼我就因此被取代了？

這時 fork 就要派上用場了！

**`fork` 這一個函式的功用就是完整地複製一份 Process** ，不論是他們的 Calling stack、Register、File descriptor 等等全部都複製一份，然後回傳 Process ID。這裡，我們說完整的意思是這二個 Process 的執行的狀態 (Global Variable、Local Variable、Function Call 等等) 幾乎可以直接互換。而 File Descriptor 也會被 dup 一份。

唯一的不同就是呼叫 `fork` 的 Process，也就是正本，其 `fork` 函式的回傳值會是複本的 Process ID。而複本的 fork 的回傳值一定是 `0`。(如果沒有錯誤產生的話)

我們可以把 `fork` 想像成一個很強大的人類複製機。今天 LoganChien 可以 `fork` 出一個 LxxxxCxxxx，他們二個不論是 DNA、記憶、外型都一樣。甚至包含按下複製前一刻的記憶都一樣。唯一的不同就是我知道複製人是 LxxxxCxxxx，而 LxxxxCxxxx 只知道他是一個複製人，不知道他是由 LoganChien 複製出來的。

為什麼要這樣設計？答案很簡單！因為不這樣設計，LxxxxCxxxx 不就造反了？LoganChien 說 LxxxxCxxxx 是他的複製人，LxxxxCxxxx 說LoganChien 是他的複製人。

而作業系統中的 Process 也是這樣。我們 `fork` 之後，會產生二個 Process。基本上這二個 Process 是很像是，除了 Process ID 與 `fork` 的回傳值，基本上這二個 Process 是一樣的！所以為了讓 Parent Process 可以掌控 Child Process，所以 Parent Process 的 `fork` 回傳值是 Child Process 的 Process ID。而 Child Process 只會得到 `0`。

這樣設計還有另一個用意，我們可以再同一份程式碼處理 Parent Process 與 Child Process 的情況。我們通常是這樣寫 `fork` 的函式呼叫的：

```c
/* 程式碼: fork.c */

#include <stdio.h>
#include <stdlib.h>

#include <sys/wait.h> /* Required for wait() */
#include <unistd.h>

int main()
{
    pid_t proc = fork(); /* Create Child Process */

    if (proc < 0) /* Failed to Create Child Process */
    {printf("Error: Unable to fork.\n");
        exit(EXIT_FAILURE);
    }
    else if (proc == 0) /* In the Child Process */
    {printf("In the child process.\n");
        exit(25);
    }
    else /* In the Parent Process */
    {printf("In the parent process.\n");

        int status = -1;
        wait(&status); /* Wait for Child Process */

        printf("The Child Process Returned with %d\n", WEXITSTATUS(status));
    }

    return EXIT_SUCCESS;
}
```
我們會先用 `fork` 建立一個 Child Process。之後二個 Process 就是各跑各的，除非 Parent Process 用 wait 來等待 Child Process 執行完畢。當然，照 POSIX API 的慣例，`fork()` 回傳負的值，就代表有錯誤發生。


在我們有了 `fork` 與 `exec*` 之後，我們就可以執行、呼叫另一個可執行檔了！原理很簡單，我們只要在 Child Process 呼叫 `exec*` 就沒有問題了。範例如下：

```c
#include <stdio.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <unistd.h>

int main()
{
    printf("Let's invoke ls command!\n\n");

    pid_t proc = fork();

    if (proc < 0)
    {
        printf("Error: Unable to fork.\n");
        exit(EXIT_FAILURE);
    }
    else if (proc == 0)
    {
        if (execlp("ls", "ls", "-al", (char *)0) == -1)
        {printf("Error: Unable to load the executable.\n");
            exit(EXIT_FAILURE);
        }

        /* NEVER REACHED */
    }
    else
    {
        int status = -1;
        wait(&status);

        printf("\nThe exit code of ls is %d.\n", WEXITSTATUS(status));
    }

    return EXIT_SUCCESS;
}
```

## 實作 Command Interpreter 的 Pipeline：上一篇的綜合練習

看完上一篇，大家應該有能力寫一個具有 Pipeline 功能的簡單 Command Interpreter。所謂的 Command Interpreter 就像是 `bash`、`ksh`、`tcsh` 之類的東西，我們也稱之為 shell。一般而言會是你登入一個系統之後第一個執行的程式。

而我們所談論的 Pipeline 有一點像 IO redirection。例如我下達以下的指令：

```
command1 | command2 | command3
```

此時 command1 的 `stdout` 會被當作 command2 的 `stdin`；command2 的 `stdout` 會被當作 command3 的 `stdin`。而當上面的指令執行時，command1 與 command3 的標準輸出都不會顯示到螢幕上。

例如：`cat /etc/passwd` 指令是用來把 `/etc/passwd` 這一個檔案的檔案內容印到 `stdout` 上面；而 `grep username` 是從 `stdin` 讀入每一行，如果某一行有 username 就輸出該行到標準輸出。所以當他們用 pipeline 組合在一起：

```
cat /etc/passwd | grep username
```

就會變成在螢幕上顯示 `/etc/passwd` 之中含有 username 的那幾行。當然，如果靈活使用 pipeline 可以用很少的指令變化出很多功能。因此 pipeline 在 *nix 環境下是很重要的東西。你能用 `open`/`close`/`dup2`/`exec*`/`fork` 寫出一個具有Pipeline 功能的 Command Interpreter 嗎？

以下是我寫到一半到程式碼，他已經可以把使用者輸入的指令轉換成若干個可以傳給 `execvp` 的 `argv`，只剩 pipeline 的部分還沒有寫完，你可以試著寫寫看：

- Original link: [http://w.csie.org/~b97073/B/todo-pipeline-shell.c](http://w.csie.org/~b97073/B/todo-pipeline-shell.c)
- Github link: [todo-pipeline-shell.c](todo-pipeline-shell.c)

```














(防雷，按 Page Down 繼續閱讀)














```

你也可以直接下載我隨手寫的版本：

- Original link: [http://w.csie.org/~b97073/B/simple-pipeline-shell.c](http://w.csie.org/~b97073/B/simple-pipeline-shell.c)
- Github link: [simple-pipeline-shell.c](simple-pipeline-shell.c)

這一份程式碼其實沒有新得東西，就是利用先前介紹過的：IO redirection ([red.c](red.c) 使用的方法)，與使用 `fork`/`exec` 來建立 child process。

**我在執行 command1 的時候，我把他的 stdout 導向一個檔案。當他結束之後，我再把這個檔案做為 stdin 導入 command2，而 command2 的 stdout 再導入另一個檔案... 以下類推。**

我們還是看一下其中的 `creat_proc` 與 `execute_cmd_seq` 二個函式：

```c
/* Purpose: Create child process and redirect io. */
void creat_proc(char **argv, int fd_in, int fd_out)
{
    /* creat_prc 函式主要的目的是建立 child process，並且做好 IO redirection。
       它的參數有三個：argv 是將來要傳給 execvp 用的；fd_in、fd_out 分別是
       輸入輸出的 file descriptor。 */

    pid_t proc = fork();

    if (proc < 0)
    {
        fprintf(stderr, "Error: Unable to fork.\n");
        exit(EXIT_FAILURE);
    }
    else if (proc == 0)
    {
        if (fd_in != STDIN_FILENO)
        {
            /* 把 fd_in 複製到 STDIN_FILENO */
            dup2(fd_in, STDIN_FILENO);
            /* 因為 fd_in 沒有用了，就關掉他 */
            close(fd_in);
        }

        if (fd_out != STDOUT_FILENO)
        {
            /* 把 fd_out 複製到 STDOUT_FILENO */
            dup2(fd_out, STDOUT_FILENO);
            /* 因為 fd_out 沒有用了，就關掉他 */
            close(fd_out);
        }

        /* 載入可執行檔，我直接把 argv[0] 當成 executable name */
        if (execvp(argv[0], argv) == -1)
        {
            fprintf(stderr,
                    "Error: Unable to load the executable %s.\n",
                    argv[0]);

            exit(EXIT_FAILURE);
        }

        /* NEVER REACH */
        exit(EXIT_FAILURE);
    }
    else
    {
        int status;
        wait(&status); /* 等程式執行完畢 */
    }
}
```

```c
/* Purpose: Create several child process and redirect the standard output
 * to the standard input of the later process.
 */
void execute_cmd_seq(char ***argvs)
{
    int C;
    for (C = 0; C <= MAX_CMD_COUNT; ++C)
    {
        char **argv = argvs[C];
        if (!argv) { break; }

        int fd_in = STDIN_FILENO;
        int fd_out = STDOUT_FILENO;

        if (C > 0)
        {
            /* 開啟暫存檔案 */
            fd_in = open(pipeline_tmp_[C - 1], O_RDONLY);

            if (fd_in == -1)
            {
                fprintf(stderr, "Error: Unable to open pipeline tmp r.\n");
                exit(EXIT_FAILURE);
            }
        }

        if (C < MAX_CMD_COUNT && argvs[C + 1] != NULL)
        {
            /* 開啟暫存檔案 */
            fd_out = open(pipeline_tmp_[C],
                          O_WRONLY | O_CREAT | O_TRUNC,
                          0644);

            if (fd_out == -1)
            {
                fprintf(stderr, "Error: Unable to open pipeline tmp w.\n");
                exit(EXIT_FAILURE);
            }
        }

        creat_proc(argv, fd_in, fd_out);

        if (fd_in != STDIN_FILENO) { close(fd_in); }
        if (fd_out != STDOUT_FILENO) { close(fd_out); }
    }
}
```

## 直接用暫存檔案實作 pipeline 的缺點

不過上面直接用暫存檔案來達成 pipeline 有什麼缺點呢？

(1) **就是慢！**因為不過是要讓二個程式相互溝通而已，實在**沒有必要把內容寫入硬碟**。而且可能會用去為數不少的空間。例如：執行這個指令一定很花時間與硬碟空間：

```
tar c / | tar xv -C .
```

(2) command1, command2, .. commandN 只能夠**依序輪流執行**。因為如果 command1 還沒寫完，而 command2 讀得比較快，則 command2 可能誤以為 command1 的輸出已經結束了。所以為了避免資料不完整，我們只能在 command1 結束之後再執行 command2。然而這樣可能比較浪費時間。


那有沒有解決的方法呢？這就是我們下一個要介紹的系統呼叫：`pipe()`。

## pipe：二個 Process 之間溝通的橋樑

`pipe` 顧名思意就是水管的意思，當我們呼叫 `pipe` 的時候，他會為我們 **開啟二個 File descriptor，一個讓我們寫入資料，另一個讓我們讀出資料。他的主要用途是讓二個 Process 可以互相溝通 (Interprocess Communication, IPC)。** 在大多數的系統中，`pipe` 是使用記憶體來當 buffer，所以會比直接把檔案寫到硬碟有效率。`pipe` 的函式原型如下：

```
int pipe(int fds[2]);
```

當我們呼叫 `pipe` 的時候，我們必需傳入一個大小 **至少為 `2` 的 `int` 陣列，`pipe` 會在 `fds[0]` 回傳一個 Read Only 的 File descriptor，在 `fds[1]` 回傳一個 Write Only 的 File descriptor。** 當二個 Processs 要相互溝通的時候，就直接使用 `write` 系統呼叫把資料寫進 `pipe`，而接收端就可以用 `read` 來讀取資料。

另外，和一般的檔案不同，除非 `pipe` 的 write-end (寫入端) 全部都被 `close` 了，不然 `read` 會一直等待新的輸入，而不是以為已經走到 eof。

> 備註：雖然我們是從 Pipeline 開始提到 `pipe()`，不過，Pipeline 未必要用 `pipe()` 實作。`pipe()` 的應用領域也不限於 Pipeline。不過以 `pipe() ` 實作 Pipeline 確實是一個很有效率的方法，究我所知，GNU bash 就是使用 `pipe()` 來實作 Pipeline。

我們可以看一下一個簡單的 Multiprocess Random Generator 的範例：

```c
/* 程式碼： pipe-example.c */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <unistd.h>

enum { RANDOM_NUMBER_NEED_COUNT = 10 };


int main()
{
    int pipe_fd[2];

    if (pipe(pipe_fd) == -1) /* 建立 pipe */
    {fprintf(stderr, "Error: Unable to create pipe.\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid;

    if ((pid = fork()) <0) /* 注意：fork 的時候，pipe 的 fd 會被 dup */
    {fprintf(stderr, "Error: Unable to fork process.\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        /* -- In the Child Process -------- */

        /* Close Read End */
        close(pipe_fd[0]); /* close read end, since we don't need it. */
        /* 我們在 Child Process 只想要當寫出端，所以我們就要先把 pipe 的 read
           end 關掉 */

        /* My Random Number Generator */
        srand(time(NULL));

        int i;
        for (i = 0; i < RANDOM_NUMBER_NEED_COUNT; ++i)
        {
            sleep(1); // wait 1 second

            int randnum = rand() % 100;
            /* 把資料寫出去 */
            write(pipe_fd[1], &randnum, sizeof(int));
        }

        exit(EXIT_SUCCESS);
    }
    else
    {
        /* -- In the Parent Process -------- */

        /* Close Write End */
        close(pipe_fd[1]); /* Close write end, since we don't need it. */
        /* 不會用到 Write-end 的 Process 一定要把 Write-end 關掉，不然 pipe
           的 Read-end 會永遠等不到 EOF。 */

        int i;
        for (i = 0; i < RANDOM_NUMBER_NEED_COUNT; ++i)
        {
            int gotnum;
            /* 從 Read-end 把資料拿出來 */
            read(pipe_fd[0], &gotnum, sizeof(int));

            printf("got number : %d\n", gotnum);
        }
    }

    return EXIT_SUCCESS;
}
```

雖然上面的例子展示了二個 Process 之間如何溝通。不過只看這個例子看不出 `pipe` 的價值。我們的第二個例子就是要利用 `pipe` 來攔截另一個 Program 的 standard output。

在第二個例子之中，我們會有二個 Program，也就是會有二個可執行檔案。其中一個專門付負製造 Random Number，然後直接把 32-bit int 寫到 standard output。而令一個會去呼叫前述的 Random Number製造程式，然後攔截他的 standard output。

```c
/* 程式碼： random-gen.c */
/* 這一個檔案就沒有什麼特別的，就只是不斷製造 Random Number */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>

enum { RANDOM_NUMBER_NEED_COUNT = 10 };

int main()
{
    srand(time(NULL));

    int i;
    for (i = 0; i < RANDOM_NUMBER_NEED_COUNT; ++i)
    {
        sleep(1); /* Wait 1 second.  Simulate the complex process of
                     generating the safer random number. */

        int randnum = rand() % 100;
        write(STDOUT_FILENO, &randnum, sizeof(int));
        /* 注意：是寫到 stdout 。*/
    }

    return EXIT_SUCCESS;
}
```

```c
/* 程式碼：pipe-example-2.c */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

enum { RANDOM_NUMBER_NEED_COUNT = 10 };

int main()
{
    /* -- Prepare Pipe -------- */
    int pipe_fd[2];

    if (pipe(pipe_fd) == -1)
    {
        fprintf(stderr, "Error: Unable to create pipe.\n");
        exit(EXIT_FAILURE);
    }


    /* -- Create Child Process -------- */
    pid_t pid;
    if ((pid = fork()) <0)
    {
        fprintf(stderr, "Error: Unable to create child process.\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) /* In Child Process */
    {
        /* Close Read End */
        close(pipe_fd[0]); /* Close read end, since we don't need it. */

        /* Bind Write End to Standard Out */
        dup2(pipe_fd[1], STDOUT_FILENO);
        /* 把第 pipe_fd[1] 個 file descriptor 複製到第 STDOUT_FILENO 個
           file descriptor */

        /* Close pipe_fd[1] File Descriptor */
        close(pipe_fd[1]);
        /* 說明：經過上面三個步驟之後，這個 Child Process 的第 1 號 File
           Descriptor 會是 pipe 的 Write-end，所以在我們做標準輸出的時候，
           所有的資料都跑進我們的 pipe 裡面。因此另一端的 Read-end 就可以
           接收到 random-gen 的標準輸出。 */

        /* Load Another Executable */
        execl("random-gen", "./random-gen", (char *)0);

        /* This Process Should Never Go Here */
        fprintf(stderr, "Error: Unexcept flow of control.\n");
        exit(EXIT_FAILURE);
    }
    else /* In Parent Process */
    {
        /* Close pipe_fd[1] File Descriptor */
        close(pipe_fd[1]); /* Close write end, since we will not use it. */

        /* Read Random Number From Pipe */
        int i;
        for (i = 0; i < RANDOM_NUMBER_NEED_COUNT; ++i)
        {
            int gotnum = -1;
            read(pipe_fd[0], &gotnum, sizeof(int));

            printf("got number : %d\n", gotnum);
        }
    }

    return EXIT_SUCCESS;
}
```

## 再回頭寫 Command Interpreter：加上 pipe() 系統呼叫，你可以寫得更好嗎？

這是我寫得另一個版本 (使用 pipe() 的版本)：

- Original link: [http://w.csie.org/~b97073/B/faster-pipeline-shell.c](http://w.csie.org/~b97073/B/faster-pipeline-shell.c)
- Github link: [faster-pipeline-shell.c](faster-pipeline-shell.c)

這次我先檢查指令有多少個 '|'，這代表我要準備多少的 `pipe`。接著我為每一個 commandI 都用 `fork` 建立一個 Process，讓所有的Process 可以用時執行。

另外，使用 `pipe()` 來實作有一個好處，就是如果 command2 要 `read` 東西，可是 command1 還沒有算完，command2 的 `read` 就會一直等下去。所以我們不用依序輪流執行。所有的 process 可以並行運作，除非遇到 IO blocking。而且使用 `pipe()` 也省去了暫存檔案命名的困擾。

但是寫 `pipe` 的版本就要注意：對於所有的 Process，如果該 Process 不需要 Write-end 就一定要記得關掉他，不然像是 `cat` 或者 `grep` 的程式就會一直等不到 EOF，也就不會結束了！

我們可以快速地看一下 `execute_cmd_seq` 與 `creat_proc` 二個函式：

```c
/* Purpose: Create several child process and redirect the standard output
 * to the standard input of the later process.
 */
void execute_cmd_seq(char ***argvs)
{
    int C, P;

    int cmd_count = 0;
    while (argvs[cmd_count]) { ++cmd_count; }

    int pipeline_count = cmd_count - 1;

    int pipes_fd[MAX_CMD_COUNT][2];

    /* 準備足夠的 pipe */
    for (P = 0; P < pipeline_count; ++P)
    {
        if (pipe(pipes_fd[P]) == -1)
        {
            fprintf(stderr, "Error: Unable to create pipe. (%d)\n", P);
            exit(EXIT_FAILURE);
        }
    }

    for (C = 0; C < cmd_count; ++C)
    {
        int fd_in = (C == 0) ? (STDIN_FILENO) : (pipes_fd[C - 1][0]);
        int fd_out = (C == cmd_count - 1) ? (STDOUT_FILENO) : (pipes_fd[C][1]);

        /* 呼叫下面的 creat_proc 來建立 Child Process */
        creat_proc(argvs[C], fd_in, fd_out, pipeline_count, pipes_fd);
    }

    /* 在建立所有 Child Process 之後，Parent Process 本身就不必使用 pipe
       了，所以關閉所有的 File descriptor。*/
    for (P = 0; P < pipeline_count; ++P)
    {
        close(pipes_fd[P][0]);
        close(pipes_fd[P][1]);
    }

    /* 等待所有的程式執行完畢 */
    for (C = 0; C < cmd_count; ++C)
    {
        int status;
        wait(&status);
    }
}
```

```c
/* Purpose: Create child process and redirect io. */
void creat_proc(char **argv,
                int fd_in, int fd_out,
                int pipes_count, int pipes_fd[][2])
{
    pid_t proc = fork();

    if (proc < 0)
    {
        fprintf(stderr, "Error: Unable to fork.\n");
        exit(EXIT_FAILURE);
    }
    else if (proc == 0)
    {
        /* 把 fd_in 與 fd_out 分別當成 stdin 與 stdout。 */
        if (fd_in != STDIN_FILENO) { dup2(fd_in, STDIN_FILENO); }
        if (fd_out != STDOUT_FILENO) { dup2(fd_out, STDOUT_FILENO); }

        /* 除了 stdin, stdout 之外，所有的 File descriptor (pipe) 都要關閉。*/
        int P;
        for (P = 0; P < pipes_count; ++P)
        {
            close(pipes_fd[P][0]);
            close(pipes_fd[P][1]);
        }

        if (execvp(argv[0], argv) == -1)
        {
            fprintf(stderr,
                    "Error: Unable to load the executable %s.\n",
                    argv[0]);

            exit(EXIT_FAILURE);
        }

        /* NEVER REACH */
        exit(EXIT_FAILURE);
    }
}
```

## 結語

我們從一個簡單的 io redirect 程式談起。一路介紹了 `exec`, `fork`, `dup2`, `pipe` 等系統呼叫。還寫了一個簡單的 Command Interpreter。
希望可以透過這二篇小小的篇幅，讓大家能對上面四個系統呼叫更為熟悉。

> 備註：這二篇大部分的程式碼可以在以下的網址取得：

[http://w.csie.org/~b97073/B/sp-article2.tar.gz](http://w.csie.org/~b97073/B/sp-article2.tar.gz)
