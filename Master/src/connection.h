#ifndef CONNECTION_H_
#define CONNECTION_H_

void connect_to_yama(void);

void request_job_for_file(const char *file);

void connect_to_worker(const char *ip, const char *port);

#endif /* CONNECTION_H_ */
