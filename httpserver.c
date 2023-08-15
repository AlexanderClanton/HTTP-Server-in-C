#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <assert.h>
#include "asgn2_helper_funcs.h"
//structs here
typedef struct {
    int in_fd;
    char cmd[2048];
    char RL[2048];
    char header[2048];
    char totbuf[2048];
    int conL;
    int status;
    long unsigned Clen;
    char crap[2048];
    char path[2048];
    long unsigned Clen2;
    long unsigned crapL;
    long unsigned marked;
} Request;
int header(Request *head);
int parsing(Request *head);
int output(Request *head);
int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Invalid Port\n");
        return 1;
    }

    int port = atoi(argv[1]);
    if (port < 1 || port > 65535) {
        fprintf(stderr, "Invalid Port\n");
        return 1;
    }

    Listener_Socket sock;

    int ret = listener_init(&sock, port);

    if (ret == -1) {
        fprintf(stderr, "Invalid Port\n");
        return 1;
    }
    bool end = true;
    int clientfd;
    int headR;

    while (end) {
        clientfd = listener_accept(&sock);

        if (clientfd != -1) {

            Request head;

            head.in_fd = clientfd;
            headR = header(&head);
            if (headR == 0) {
                headR = parsing(&head);
                if (headR == 0) {
                    output(&head);
                }
            }
            close(head.in_fd);
        }
    }

    return 0;
}

int header(Request *head) {
    printf("in head");
    ssize_t byteR;

    char end[5];
    end[0] = '\r';
    end[1] = '\n';
    end[2] = '\r';
    end[3] = '\n';
    end[4] = '\0';

    byteR = read_until(head->in_fd, head->totbuf, 2048, end);
    head->Clen = byteR;
    if (byteR == -1) {
        head->status = 400;
        char bufH8[2048] = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";
        write_all(head->in_fd, bufH8, strlen(bufH8));
        return 1;
    }
    bool truth = false;
    //fprintf(stderr,"buf is %s\n",buf);
    //int len = strlen(buf);
    // buf[byteR] = '\0';
    //fprintf(stderr,"buf2 is %s\n",buf);
    head->totbuf[byteR] = '\0';
    for (unsigned long zz = 0; zz < head->Clen - 3; zz++) {
        if (head->totbuf[zz] == '\r' && head->totbuf[zz + 1] == '\n' && head->totbuf[zz + 2] == '\r'
            && head->totbuf[zz + 3] == '\n') {
            head->marked = zz + 4;
            truth = true;
            break;
        }
    }

    if (truth == false) {
        head->status = 400;
        char bufH8[2048] = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";
        write_all(head->in_fd, bufH8, strlen(bufH8));
        return 1;
    }
    return 0;
}

int parsing(Request *head) {
    char buf[2048] = "";
    strcpy(buf, head->totbuf);
    char buf2[2048] = "";
    char buf9[2048] = "";
    strcpy(buf2, head->totbuf);
    strcpy(buf9, head->totbuf);
    char *hold = NULL;
    char *holdM = NULL;
    hold = strtok_r(buf, "\r\n", &holdM);
    head->crapL = strlen(hold);
    head->crapL += 2;
    /*
    for (unsigned long zz = 0; zz < head->Clen - 3; zz++) {
        if (head->totbuf[zz] == '\r' && head->totbuf[zz + 1] == '\n' && head->totbuf[zz + 2] == '\r'
            && head->totbuf[zz + 3] == '\n') {
            head->marked = zz + 4;
            break;
        }
    }
	*/
    //	fprintf(stderr," actual len %lu\n crap is %lu\n",head->Clen,head->marked);

    //fprintf(stderr,"hold end $$%s$$\n",hold3);

    //first block here tests regex for 3 split
    //then put the get/put hopefully
    //if dont match, whole thing, give em 400
    regex_t re;
    regmatch_t matches[4];
    int rc;
    rc = regcomp(&re, "^([A-Z]{3,8}) /([a-zA-Z0-9.-]{1,64}) (HTTP/[0-9]\\.[0-9])$", REG_EXTENDED);
    assert(!rc);
    rc = regexec(&re, (char *) hold, 4, matches, 0);
    if (rc != 0) {
        fprintf(stderr, "weird\n");
        //insert status crap
        //deal with http 1.1
        char buf3[2048] = "";
        char buf4[2048] = "";
        strcpy(buf3, hold);
        char *hold2;
        char *holdm2;
        hold2 = strtok_r(buf3, " ", &holdm2);
        hold2 = strtok_r(NULL, " ", &holdm2);
        hold2 = strtok_r(NULL, "", &holdm2);
        strncpy(buf4, hold2, 4);
        //fprintf(stderr,"http looks like %s",buf4);
        /*
		if(strcmp(buf4,"HTTP")==0){
			head->status=505;
			char buf5[2048]="HTTP/1.1 505 Version Not Supported\r\nContent-Length: 22\r\n\r\nVersion Not Supported\n";
			write_all(head->in_fd,buf5,strlen(buf5));
			return 1;
		}
		*/
        head->status = 400;
        char bufH8[2048] = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";
        write_all(head->in_fd, bufH8, strlen(bufH8));
        return 1;
    }

    //fprintf(stderr,"hold total is %s\n",hold);
    strcpy(head->cmd, hold);
    head->cmd[matches[1].rm_eo] = '\0';

    //fprintf(stderr,"cmd is %s\n",head->cmd);
    strcpy(head->path, hold + matches[2].rm_so);
    head->path[matches[2].rm_eo - matches[2].rm_so] = '\0';
    //fprintf(stderr,"path is %s\n",head->path);
    strcpy(head->RL, hold + matches[3].rm_so);
    head->RL[matches[3].rm_eo - matches[3].rm_so] = '\0';
    //fprintf(stderr,"RL http is $$%s$$\n",head->RL);

    regfree(&re);

    if (head->RL[5] != '1' || head->RL[7] != '1') {
        head->status = 505;
        char buf5[2048] = "HTTP/1.1 505 Version Not Supported\r\nContent-Length: 22\r\n\r\nVersion "
                          "Not Supported\n";
        write_all(head->in_fd, buf5, strlen(buf5));
        return 1;
    }

    //second get me the path

    //then check http actually 1.1

    //next its header regex for each, only want content. but shit must be valid

    if (strcmp(head->cmd, "PUT") == 0) {
        hold = strtok_r(NULL, "\r\n", &holdM);
        //fprintf(stderr,"hold is%s\n",hold);
        head->crapL += strlen(hold);
        head->crapL += 2;
        if (strcmp(hold, "\r\n") == 0) {
            head->status = 400;
            char bufH8[2048]
                = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";
            write_all(head->in_fd, bufH8, strlen(bufH8));
            return 1;
        }
        char CLL[2048] = "";
        char CLL2[2048] = "";
        int rc2;
        regex_t re2;
        rc2 = regcomp(&re2, "^([a-zA-Z0-9.-]{1,128}): ([[:print:]]{0,128})$", REG_EXTENDED);
        assert(!rc2);
        bool truthZ2 = false;
        bool truthZ = true;
        while (truthZ) {

            regmatch_t matcheZ[3];
            rc2 = regexec(&re2, (char *) hold, 3, matcheZ, 0);
            if (rc2 != 0) {
                fprintf(stderr, "got here\n");

                head->status = 400;
                char bufH8[2048]
                    = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";
                write_all(head->in_fd, bufH8, strlen(bufH8));
                return 1;
            }
            strcpy(CLL2, hold);
            CLL2[matcheZ[1].rm_eo] = '\0';
            //fprintf(stderr,"cl is %s\n",CLL2);
            strcpy(CLL, hold + matcheZ[2].rm_so);
            CLL[matcheZ[2].rm_eo - matcheZ[2].rm_so] = '\0';
            //fprintf(stderr,"cl2 is %s\n",CLL);
            if (strcmp(CLL2, "Content-Length") == 0) {
                head->Clen2 = atoi(CLL);
                truthZ2 = true;
            } else if (strcmp(CLL2, "content-length") == 0) {
                head->Clen2 = atoi(CLL);
                truthZ2 = true;
            }
            hold = strtok_r(NULL, "\r\n", &holdM);

            if (hold != NULL) {

                head->crapL += strlen(hold);
                head->crapL += 2;
                //fprintf(stderr,"values are real %d\n crap %d",head->marked,head->crapL);
            }
            if (head->crapL > head->marked) {
                truthZ = false;

                break;
            }

            //fprintf(stderr,"hold is $$%s$$\n",hold);
            //if(hold!=NULL){
            //	head->crapL+=strlen(hold);
            //}

            if (hold == NULL) {
                fprintf(stderr, "not here");
                if (truthZ2 == false) {
                    head->status = 400;
                    char bufH8[2048]
                        = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";
                    write_all(head->in_fd, bufH8, strlen(bufH8));
                    regfree(&re2);
                    return 1;
                }

                truthZ = false;
            }
        }
        regfree(&re2);
    } else if (strcmp(head->cmd, "GET") == 0) {
        /*
		hold=strtok_r(NULL,"\r\n",&holdM);
		if(strcmp(hold,"\r\n")!=0){
			head->status = 400;
            char bufH8[2048] = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";
            write_all(head->in_fd, bufH8, strlen(bufH8));
            return 1;	
		}
		*/
        // head->totbuf[head->Clen]='\0';
        hold = strtok_r(NULL, "\r\n", &holdM);

        int rc2;
        regex_t re2;
        rc2 = regcomp(&re2, "^([a-zA-Z0-9.-]{1,128}): ([[:print:]]{0,128})$", REG_EXTENDED);
        assert(!rc2);
        bool truthZ = true;
        while (truthZ) {
            regmatch_t matcheZ[3];
            rc2 = regexec(&re2, (char *) hold, 3, matcheZ, 0);
            if (rc2 != 0) {
                //  fprintf(stderr,"got here\n");

                head->status = 400;
                char bufH8[2048]
                    = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";
                write_all(head->in_fd, bufH8, strlen(bufH8));
                return 1;
            }

            hold = strtok_r(NULL, "\r\n", &holdM);
            if (hold == NULL) {
                truthZ = false;
            }
        }

        regfree(&re2);
    }
    //head->crapL+=(trackNL*2);
    //crap section

    //int tot_b=0;
    //fprintf(stderr,"$%s",buf2);
    //hold5=strtok_r(buf2,"\r\n\r\n",&hold6);
    //tot_b+=strlen(hold5);
    //hold5=strtok_r(NULL,"",&hold6);
    //tot_b+=strlen(hold5);
    //fprintf(stderr,"hold$$ crap is %s",hold5);
    //hold5=strtok_r(NULL,"",&hold6);
    /*
	if(hold5!=NULL){
		fprintf(stderr,"hold crap is %s",hold5);
		
		strcpy(head->crap,hold5);
	}
	else if(hold5==NULL){
	//	fprintf(stderr,"\nLOL\n");
	}
	//fprintf(stderr,"\nAHHHHH\n");	
	*/

    head->crapL = 0;
    unsigned long lenC = 0;
    unsigned long lenC2 = head->marked;

    if (lenC2 < head->Clen) {
        //fprintf(stderr,"not here %c $$%c &&%c&&\n",head->totbuf[lenC2-1],head->totbuf[lenC2],head->totbuf[lenC2+1]);

        for (lenC = 0; lenC < (head->Clen - head->marked); lenC++) {
            head->crap[lenC] = head->totbuf[lenC2];
            head->crapL += 1;
            lenC2 += 1;
        }

        /*
		fprintf(stderr,"extra test $$%c$$\n",head->totbuf[lenC2]);
		strncpy(CHECK,head->totbuf,(head->marked));
		strcat(CHECK,head->crap);
		if(strcmp(CHECK,head->totbuf)==0){
			fprintf(stderr,"yes!\n");
			fprintf(stderr, "cap length is $$%lu$$ tot buf is  &&%d&& marked is **%lu**",head->crapL, head->Clen, head->marked);
		}
		*/
    }
    //fprintf(stderr,"crap length is %lu",head->crapL);
    //fprintf(stderr,"%s",head->crap);

    //fprintf(stderr, "GOT HERE\n");
    return 0;
}

int output(Request *head) {
    //read the rest, add back stuff
    //use values of either printing or setting file
    struct stat sb;
    //fprintf(stderr,"COM &&%s&&\n",head->cmd);
    if (strcmp(head->cmd, "GET") == 0) {
        //  fprintf(stderr,"not here?\n");
        char bufH[2048] = "";
        char str[20 + sizeof(char)];
        strcpy(bufH, head->path);
        if (access(bufH, F_OK) == 0) {

            int fd2;
            if ((open(bufH, O_DIRECTORY, 0666)) > 0) {
                head->status = 403;
                char bufH8[2048]
                    = "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n";
                write_all(head->in_fd, bufH8, strlen(bufH8));
                return 1;
            }
            fd2 = open(bufH, O_RDONLY, 0664);
            if (fd2 < 0) {
                //	fprintf(stderr,"path is %s\n RL is %s\n crap is %s\n header is %s\n length is %lu\n",head->path,head->RL,head->crap,head->header, head->Clen2);
                head->status = 403;
                char bufH8[2048]
                    = "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n";
                write_all(head->in_fd, bufH8, strlen(bufH8));
                return 1;

            } else {
                //  fprintf(stderr,"WHY NOT HERE\n");
                char bufH2[2048] = "HTTP/1.1 200 OK\r\nContent-Length: ";
                stat(bufH, &sb);
                long size = sb.st_size;
                sprintf(str, "%lu", sb.st_size);
                strcat(bufH2, str);
                strcat(bufH2, "\r\n\r\n");
                int byteW = 0;
                int byteR = 0;

                byteW = write_all(head->in_fd, bufH2, strlen(bufH2));
                if (byteW == -1) {
                    head->status = 500;
                    //write to client why
                    char bufH7[2048] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "
                                       "1\r\n\r\nInternal Server Error\n";
                    write_all(head->in_fd, bufH7, strlen(bufH7));
                    close(fd2);
                    return 1;
                }
                byteR = pass_bytes(fd2, head->in_fd, size);
                if (byteR == -1) {
                    head->status = 500;
                    //write to client why
                    char bufH7[2048] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "
                                       "1\r\n\r\nInternal Server Error\n";
                    write_all(head->in_fd, bufH7, strlen(bufH7));
                    close(fd2);
                    return 1;
                }
                close(fd2);
            }

        } else {
            head->status = 404;
            //write shit out
            char bufH6[2048] = "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n";
            write_all(head->in_fd, bufH6, strlen(bufH6));
            return 1;
        }

    } else if (strcmp(head->cmd, "PUT") == 0) {
        char bufH[2048] = "";
        strcpy(bufH, head->path);

        int fd2;
        bool fexist = true;
        //fprintf(stderr,"IM HERE\n");
        if (access(bufH, F_OK) != 0) {
            fexist = false;
        }
        fd2 = open(bufH, O_CREAT | O_TRUNC | O_WRONLY, 0666);
        if (fd2 < 0) {
            head->status = 500;
            //write to client why
            char bufH7[2048] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "
                               "1\r\n\r\nInternal Server Error\n";
            write_all(head->in_fd, bufH7, strlen(bufH7));
            return 1;
        }

        char bufH2[2048] = "HTTP/1.1 ";
        if (fexist) {
            strcat(bufH2, "200 OK\r\nContent-Length: 3\r\n\r\nOK\n");

        } else {
            strcat(bufH2, "201 Created\r\nContent-Length: 8\r\n\r\nCreated\n");
        }

        if (head->marked != head->Clen) {
            //fprintf(stderr,"\nGOT HERE\n");
            int byteR = 0;
            byteR = write_all(fd2, head->crap, head->crapL);
            if (byteR == -1) {
                head->status = 500;
                //write to client why
                char bufH7[2048] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "
                                   "1\r\n\r\nInternal Server Error\n";
                write_all(head->in_fd, bufH7, strlen(bufH7));
                close(fd2);
                return 1;
            }
            byteR = pass_bytes(head->in_fd, fd2, (head->Clen2 - (head->Clen - head->marked)));
            if (byteR == -1) {
                head->status = 500;
                //write to client why
                char bufH7[2048] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "
                                   "1\r\n\r\nInternal Server Error\n";
                write_all(head->in_fd, bufH7, strlen(bufH7));
                close(fd2);
                return 1;
            }

            byteR = write_all(head->in_fd, bufH2, strlen(bufH2));
            if (byteR == -1) {
                head->status = 500;
                //write to client why
                char bufH7[2048] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "
                                   "1\r\n\r\nInternal Server Error\n";
                write_all(head->in_fd, bufH7, strlen(bufH7));
                close(fd2);
                return 1;
            }
        } else {
            //write_all(fd2,head->crap,head->Clen-(head->marked));
            int byteR = 0;
            byteR = pass_bytes(head->in_fd, fd2, head->Clen2);
            if (byteR == -1) {
                head->status = 500;
                //write to client why
                char bufH7[2048] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "
                                   "1\r\n\r\nInternal Server Error\n";
                write_all(head->in_fd, bufH7, strlen(bufH7));
                close(fd2);
                return 1;
            }
            byteR = write_all(head->in_fd, bufH2, strlen(bufH2));
            if (byteR == -1) {
                head->status = 500;
                //write to client why
                char bufH7[2048] = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: "
                                   "1\r\n\r\nInternal Server Error\n";
                write_all(head->in_fd, bufH7, strlen(bufH7));
                close(fd2);
                return 1;
            }
        }

        close(fd2);

    } else if (strcmp(head->cmd, "CONNECT") == 0) {
        head->status = 501;
        char bufH2[2048]
            = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n";
        write_all(head->in_fd, bufH2, strlen(bufH2));
        return 1;
    } else if (strcmp(head->cmd, "DELETE") == 0) {
        head->status = 501;
        char bufH2[2048]
            = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n";
        write_all(head->in_fd, bufH2, strlen(bufH2));
        return 1;

    } else if (strcmp(head->cmd, "HEAD") == 0) {
        head->status = 501;
        char bufH2[2048]
            = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n";
        write_all(head->in_fd, bufH2, strlen(bufH2));
        return 1;
    } else if (strcmp(head->cmd, "OPTIONS") == 0) {
        head->status = 501;
        char bufH2[2048]
            = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n";
        write_all(head->in_fd, bufH2, strlen(bufH2));
        return 1;
    } else if (strcmp(head->cmd, "POST") == 0) {
        head->status = 501;
        char bufH2[2048]
            = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n";
        write_all(head->in_fd, bufH2, strlen(bufH2));
        return 1;
    } else if (strcmp(head->cmd, "TRACE") == 0) {
        head->status = 501;
        char bufH2[2048]
            = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n";
        write_all(head->in_fd, bufH2, strlen(bufH2));
        return 1;
    } else {
        head->status = 501;
        char bufH2[2048]
            = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n";
        write_all(head->in_fd, bufH2, strlen(bufH2));
        return 1;

        /*
        head->status = 400;
        char bufH2[2048] = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";
        write_all(head->in_fd, bufH2, strlen(bufH2));
        return 1;
		*/
    }

    return 0;
}
