#include "MultipartParser.h"
// #include "MultipartReader.h"
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>

#define TEST_PARSER
#define INPUT_FILE "input.txt"
#define BOUNDARY "abcd"
// #define BOUNDARY "-----------------------------168072824752491622650073"
#define TIMES 1
// #define SLURP
// #define QUIET

using namespace std;

// 先找到前端的分隔符，然后再用解析器分割
int main()
{
#ifdef TEST_PARSER
	MultipartParser parser;
#endif

	struct timeval stime, etime;
	struct stat sbuf;

	stat(INPUT_FILE, &sbuf);

	size_t bufsize = 1024 * 32;
	char *buf = (char *)malloc(bufsize);

	gettimeofday(&stime, NULL);
	for (int i = 0; i < TIMES; i++)
	{
#ifndef QUIET
		printf("------------\n");
#endif
		parser.setBoundary(BOUNDARY);

		FILE *f = fopen(INPUT_FILE, "rb");
		while (!parser.stopped() && !feof(f))
		{
			size_t len = fread(buf, 1, bufsize, f);
			size_t fed = 0;
			do
			{
				size_t ret = parser.feed(buf + fed, len - fed);
				fed += ret;
			} while (fed < len && !parser.stopped());
		}
        parser.genReflectData();

        for(auto iter : parser.bodyData){
            printf("body测试: %s\n", iter.c_str());
        }
        for(auto iter : parser.reflectData){
            printf("k v测试: %s, %s\n", iter.first.c_str(), iter.second.c_str());
        }
#ifndef QUIET
		printf("%s\n", parser.getErrorMessage());
#endif
		fclose(f);
	}
	gettimeofday(&etime, NULL);

	unsigned long long a = (unsigned long long)stime.tv_sec * 1000000 + stime.tv_usec;
	unsigned long long b = (unsigned long long)etime.tv_sec * 1000000 + etime.tv_usec;
	printf("(C++)    Total: %.2fs   Per run: %.2fs   Throughput: %.2f MB/sec\n",
		   (b - a) / 1000000.0,
		   (b - a) / TIMES / 1000000.0,
		   ((unsigned long long)sbuf.st_size * TIMES) / ((b - a) / 1000000.0) / 1024.0 / 1024.0);

	return 0;
}
