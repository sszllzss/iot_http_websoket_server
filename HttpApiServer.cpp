/*************************************************************************
# > File Name: HttpApiServer.cpp
# > Author: SSZL
# > Mail: sszllzss@foxmail.com
# > Blog: sszlbg.cn
# > Created Time: 2018-10-07 22:14:33
# > Revise Time: 2018-10-08 16:55:35
 ************************************************************************/

#include<iostream>
#include <signal.h>
#include "Hppt_WebSocket/include/ev_httpd.h"
#include "Hppt_WebSocket/include/ev_httpd_content.h"
#include "CJsonObject/CJsonObject.hpp"
#include <event2/event.h>
using namespace std;

const char *getFileType(const char *filename){ //根据扩展名返回文件类型描述  
    char sExt[32];
    const char *p_start = filename + strlen(filename) - 1;
    memset(sExt, 0, sizeof(sExt));
    while (*p_start)
    {
        if (*p_start == '.')
        {
            p_start++;
            strncpy(sExt, p_start, sizeof(sExt));
            break;

        }
        p_start--;

    }

    if (strncmp(sExt, "bmp", 3) == 0)
        return "image/bmp";

    if (strncmp(sExt, "gif", 3) == 0)
        return "image/gif";

    if (strncmp(sExt, "ico", 3) == 0)
        return "image/x-icon";

    if (strncmp(sExt, "jpg", 3) == 0)
        return "image/jpeg";

    if (strncmp(sExt, "avi", 3) == 0)
        return "video/avi";

    if (strncmp(sExt, "css", 3) == 0)
        return "text/css";

    if (strncmp(sExt, "dll", 3) == 0)
        return "application/x-msdownload";

    if (strncmp(sExt, "exe", 3) == 0)
        return "application/x-msdownload";

    if (strncmp(sExt, "dtd", 3) == 0)
        return "text/xml";

    if (strncmp(sExt, "mp3", 3) == 0)
        return "audio/mp3";

    if (strncmp(sExt, "mpg", 3) == 0)
        return "video/mpg";

    if (strncmp(sExt, "png", 3) == 0)
    {
        return "image/png";
    }

    if (strncmp(sExt, "ppt", 3) == 0)
        return "application/vnd.ms-powerpoint";

    if (strncmp(sExt, "xls", 3) == 0)
        return "application/vnd.ms-excel";

    if (strncmp(sExt, "doc", 3) == 0)
        return "application/msword";

    if (strncmp(sExt, "mp4", 3) == 0)
        return "video/mpeg4";

    if (strncmp(sExt, "ppt", 3) == 0)
        return "application/x-ppt";

    if (strncmp(sExt, "wma", 3) == 0)
        return "audio/x-ms-wma";

    if (strncmp(sExt, "wmv", 3) == 0)
        return "video/x-ms-wmv";
    if(strncmp(sExt, "wmv", 3) == 0)
        return "application/x-javascript";
    return "text/html";
}
int getFileSize(const char * filename)
{
    int fileSize = 0;
    FILE *fp = NULL;
    if (filename == NULL)
        return 0;
    //以二进制的形式读取
    fp = fopen(filename, "rb");
    if (fp == NULL)
        return 0;
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    rewind(fp);
    fclose(fp);
    return fileSize;
}

void serve_file(struct httpChilent_t *  client, const char *filename)
{
    int fp = open(filename,O_RDONLY|O_NONBLOCK);
    char num[20];
    if (fp == -1)
    {
        client->resqonse.resqonseCode = HTTP_NOTFOUND;
    }
    else
    {
        int len = getFileSize(filename);
        if(len == 0)
        {
            client->resqonse.resqonseCode = HTTP_INTERNAL;
            close(fp);
            return;
        }
        sprintf(num,"%d",len);
        client->resqonse.Resqonse_data_len = len;
        client->resqonse.resqonse_data = (char *)malloc(client->resqonse.Resqonse_data_len);
        if(client->resqonse.resqonse_data == NULL)
        {
            client->resqonse.resqonseCode = HTTP_INTERNAL;
            close(fp);
            return;
        }
        client->resqonse.resqonseCode = HTTP_OK;
        client->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Type", getFileType(filename)));
        client->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Length",num));
        if(read(fp, client->resqonse.resqonse_data, client->resqonse.Resqonse_data_len) == -1)
        {
            perror("rev");
            free(client->resqonse.resqonse_data);
            client->resqonse.resqonse_data = NULL;
            client->resqonse.resqonseCode = HTTP_INTERNAL;
            close(fp);
            return;
        }
    }
    close(fp);
}
void httpd_handler(struct httpChilent_t * httpChilent) 
{
    char path[512];
    do{
        if(strcasecmp(httpChilent->request.method, "GET") == 0)
        {
            if(strcmp(httpChilent->request.url,"/") == 0 || strcmp(httpChilent->request.url, "/null") == 0)
            {
                sprintf(path, "./wwwroot/index.html");
            }
            else
            {
                sprintf(path, "./wwwroot%s", httpChilent->request.url);
                if(httpChilent->request.url[strlen(httpChilent->request.url) - 1] == '/')
                {
                    strcat(path, "index.html");
                }

            }
            serve_file(httpChilent, path);
            /*
            httpChilent->resqonse.resqonseCode = HTTP_OK;
            int len  = httpChilent->resqonse.Resqonse_data_len= strlen(HTTP_MOVEPERM_CONTENT);
            sprintf(num,"%d",len);
            httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Type", "text/html"));
            httpChilent->resqonse.HeadParameter->insert(HaedParameteMapPairi("Content-Length",num));
            httpChilent->resqonse.resqonse_data = (char *)malloc(httpChilent->resqonse.Resqonse_data_len);
            strcpy(httpChilent->resqonse.resqonse_data, HTTP_MOVEPERM_CONTENT);
            */
        }
        else if(strcasecmp(httpChilent->request.method, "POST") == 0)
        {
             
        }
        if(httpChilent->request.HeadParameter != NULL)
        {
            HeadParameterMap::iterator v;
            v = httpChilent->request.HeadParameter->find("Connection");
            if(v != httpChilent->request.HeadParameter->end())
            {
                if(strcasecmp(v->second.c_str(), "keep-alive") == 0)
                {

                }
                else if(strcasecmp(v->second.c_str(), "close") == 0)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        resqonse(httpChilent);
        return;
    }while(0);
    resqonse(httpChilent);
    httpChilent_Close(httpChilent);
}

void ws_read_cb(struct httpChilent_t * httpChilent)
{

}
void close_signal_cb(evutil_socket_t sig,short events, void *user_data)
{
    struct event_base *base = (struct event_base *)user_data;
    struct timeval delay = {2, 0};
    if(sig == SIGINT)
    {
        printf("2s后程序关闭\r\n");
        event_base_loopexit(base, &delay);
    }
}

int main(int argc, char *argv[])
{
    u_short port = 8088;
    struct event_base * base;    
    struct event * close_signal; 
    httpServer_t * httpServer;
    base = event_base_new();
    
    httpServer = startHttpServe(base, &port);
    
    httpServer_setHttpHandler(httpServer, httpd_handler);
    httpServer_setWebSocket_read_cb(httpServer, ws_read_cb, "/");
    close_signal =  evsignal_new(base,SIGINT,close_signal_cb, base); 
    if(!close_signal || event_add(close_signal, NULL) < 0)
    {
        fprintf(stderr, "Could not create/add a signal event!\n");
        exit(1);
    }
 
    event_base_dispatch(base);
    httpServer_free(httpServer);
    event_base_free(base);

    return 0;
}
