#include <iostream>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <cmath>
#include <iomanip>

using namespace std;

#define PI 3.14159265359

struct parameters
{
	int motor_pos;
	float angle;
	float focal;
	float pix2cmx;
	float pix2cmy;
	float s_view_compensation;
};

void converttotext(char filename[])
{
	char input[100];
	char output[100];

	strcpy(input,filename);
	strcpy(output,filename);
	strcat(input,".dat");
	strcat(output,".txt");

	ifstream constants(input,ios::binary);
	ofstream converted(output);

	parameters data_entry;

	while(1)
	{
		if(constants.eof())
			break;

		constants.read((char*)&data_entry,sizeof(data_entry));
		
		converted<<data_entry.motor_pos<<"\t\t\t"<<setprecision(5)<<data_entry.angle<<"\t\t\t"
				<<setprecision(5)<<data_entry.focal<<"\t\t\t"<<setprecision(5)<<data_entry.pix2cmy<<"\t\t\t"<<setprecision(5)
				<<data_entry.s_view_compensation<<"\n";
	}

	constants.close();
	converted.close();
}

int main()
{
	char x;
	char filenames[20][50];
	char input[100];
	int files=0;			//number of files

	ofstream output;
	output.open("average_constants.dat",ios::binary);

	ifstream data[20];		//maximum 20 files are supported

	do
	{
		cout<<"\nENTER FILENAMES :";
		// gets(input);
		cin>>input;
		strcat(input,".dat");
		strcpy(filenames[files],input);
		data[files].open(filenames[files],ios::binary);
		if(data[files].fail())
			cout<<"\nFILE DOES NOT EXIST";
		else
		{
			cout<<"SUCCESS";
			++files;
		}

		cout<<"\nDO YOU WANT TO CONTINUE :";
		cin>>x;
	}while(x=='Y' || x=='y');
		

	parameters entry[20];
	parameters temp;
	parameters write;
	parameters sum;
	
	while(1)
	{
		if(data[0].eof())
			break;

		int found=0;
		data[0].read((char*)&entry[0],sizeof(entry[0]));

		for(int i=1;i<files;++i)
		{
			data[i].seekg(0,ios::beg);
			while(1)
			{
				if(data[i].eof())
					break;

				data[i].read((char*)&temp,sizeof(temp));
				
				if(temp.motor_pos==entry[0].motor_pos)
				{
					entry[i]=temp;
					found++;
					cout<<"\nFOUND";
					break;
				}
			}
		}

		sum.motor_pos=0;
		sum.angle=0.0;
		sum.focal=0.0;
		sum.pix2cmy=0.0;
		sum.s_view_compensation=0.0;

		for(int i=0;i<files;++i)
		{
			sum.motor_pos+=entry[i].motor_pos;
			sum.angle+=entry[i].angle;
			sum.focal+=entry[i].focal;
			sum.pix2cmy+=entry[i].pix2cmy;
			sum.s_view_compensation+=entry[i].s_view_compensation;
		}

		if(found==files-1)			//to ensure that motor location is available in all files entered
		{							//otherwise it will not be included
			write.motor_pos=sum.motor_pos/files;
			write.angle=sum.angle/float(files);
			write.focal=sum.focal/float(files);
			write.pix2cmy=sum.pix2cmy/float(files);
			write.s_view_compensation=sum.s_view_compensation/float(files);
			cout<<"\nwriting...";
			output.write((char*)&write,sizeof(write));
		}
	}

	for(int i=0;i<files;i++)
		data[i].close();
	output.close();
	
	char convert[100]="average";
	converttotext(convert);

	return 0;
}