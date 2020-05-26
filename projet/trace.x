struct data
{
    string message <>;
    int pid;
};
typedef struct data data;

struct reponse
{
    int errno;
};
typedef struct reponse reponse;

program CALCUL
{
    version VERSION_UN
    {
        reponse TRACE(data) = 0;
    }
    = 1;
}
= 0x20000001;