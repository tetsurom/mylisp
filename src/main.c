#include <stdio.h>
#include <string.h>

FILE* open_stream(int argc, const char* argv[]);
int get_token(FILE* file, char buf[], int buf_size);


int main(int argc, const char* argv[])
{
    FILE* file = open_stream(argc, argv);
    char token[1024];
    while(0 != get_token(file, token, sizeof(token)/sizeof(char)))
    {
        printf("%s\n", token);
    }

    if(file != stdin) fclose(file);
    return 0;
}

FILE* open_stream(int argc, const char* argv[])
{
    FILE* file;
    if(argc < 2) return stdin;
    if(file = fopen(argv[1], "r"))
    {
        return file;
    }
    return stdin;
}

int get_token(FILE* file, char buf[], int buf_size)
{
    int len = 0;
    char ch = fgetc(file);
    while(ch != EOF && len < buf_size)
    {
        switch(ch)
        {
            case '(':
            case ')':
                if(len != 0)
                {
                    ungetc(ch, file);
                }
                else
                {
                    buf[len++] = ch;
                }
                goto END_OF_TOKEN;
            case '\n':
            case '\r':
            case '\t':
            case ' ':
                if(len != 0)
                {
                    goto END_OF_TOKEN;
                }
                else
                {
                    break;
                }
            default:
                buf[len++] = ch;
                break;
        }
        ch = fgetc(file);
    }
END_OF_TOKEN:
    buf[len] = '\0';
    return len;
}


