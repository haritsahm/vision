#include <opencv2/opencv.hpp>
#include <cstdio>

main(int argc, char *argv[])
{
	if(argc < 1)
	{
		printf("Usage: %s <Filename>\n", argv[0]);
		exit(1);
	}

	IplImage* frame = cvLoadImage(argv[1],CV_LOAD_IMAGE_GRAYSCALE);
	for (int i = 0; i < strlen(argv[1]); ++i)
	{		
		if(argv[1][i] == '.')
		{
			argv[1][i+1] = 'p';
			argv[1][i+2] = 'g';
			argv[1][i+3] = 'm';
			break;
		}
	}
	cvSaveImage(argv[1],frame);
	printf("IMAGE SAVED\n");
}