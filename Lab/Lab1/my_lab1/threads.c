#define CURL_STATICLIB
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <unistd.h>

//pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
int ready = 0;

static FILE *output; // make sure to `fopen` and `fclose` this in `main`!
size_t write_data(char *, size_t, size_t, void *);
void *downloader_thread(void *arg);
void *UI_thread(void *arg);
/*
int my_progress_func(char *progress_data,  
					 double t, 
					 double d, 
					 double ultotal,  
					 double ulnow); */

int main()
{
    output = fopen("file.txt", "w+");

    pthread_t p1, p2;
    pthread_create(&p1, NULL, downloader_thread, output);
    pthread_create(&p2, NULL, UI_thread, NULL);
    //  join waits for the threads to finish
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    fclose(output);
    return 0;
}

void *downloader_thread(void *arg)
{
    //pthread_mutex_lock(&lock);
    //char *progress_data = "* ";
    CURL *curl = curl_easy_init();
    if (curl)
    {
        CURLcode res;
        // change this to whatever URL you actually want to download, being
        // careful to not download something bigger than the `quota` command
        // reports as the maximum space you have in your user directory.
        curl_easy_setopt(curl, CURLOPT_URL, "https://google.com");
        //curl_easy_setopt(curl, CURLOPT_URL, "https://umanitoba.ca/");
        // use our write function (to write to a file instead of stdout)
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        // artificially limit download speeds to 2kb/s
        curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t)2048);
        /* download percent
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, my_progress_func);  
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_data); 
        */

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        ready = 1;
    }
    //pthread_mutex_unlock(&lock);
    return NULL;
}

void *UI_thread(void *arg)
{
    int counter;
    //pthread_mutex_lock(&lock2);
    printf("\ndownloading....");
    while (!ready)
    {
        for (counter = 0; counter < 4; counter++)
        {
            printf("\b%c", "|/-\\"[counter]);
            fflush(stdout);
            usleep(300000);
        }
        //pthread_cond_wait(&cond, &lock);
    }
    printf("\n\ndownload successful\n\n");
    //pthread_mutex_unlock(&lock2);
    return NULL;
}

// later
size_t write_data(char *data, size_t size, size_t nmemb, void *extra)
{
    (void)extra; // we're not using this argument at all, cast it
                 // to explicitly tell the compiler that we know it's unused.
    return fwrite(data, size, nmemb, output);
}


int my_progress_func(char *progress_data,  
					 double t, 
					 double d,  
					 double ultotal,  
					 double ulnow)  
{  
	printf("%s %g / %g (%g %%)\n", progress_data, d, t, d*100.0/t);  
	return 0;  
}
