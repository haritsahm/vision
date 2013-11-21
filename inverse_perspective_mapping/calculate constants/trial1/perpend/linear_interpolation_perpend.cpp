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
}entry[1000];

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
				<<setprecision(5)<<data_entry.focal<<"\t\t\t"<<setprecision(5)<<data_entry.pix2cmx<<"\t\t\t"
				<<setprecision(5)<<data_entry.pix2cmy<<"\t\t\t"<<setprecision(5)<<data_entry.s_view_compensation<<"\n";
	}

	constants.close();
	converted.close();
}

int main()
{
	char filename[100];
	cout<<"ENTER FILENAME TO BE INTERPOLATED (NO EXTENSION) :";
	// gets(filename);
	cin>>filename;
	char datafile[100]="";
	char outputfile[100]="";
	strcpy(datafile,filename);
	strcpy(outputfile,filename);
	strcat(datafile,".dat");
	strcat(outputfile,"_interpolated.dat");

	ifstream data;
	ofstream output;

	data.open(datafile,ios::binary|ios::in);
	if(data.is_open())
		cout<<"\nDATAFILE OPENED SUCCESSFULLY";
	else
		cout<<"\nUNABLE TO OPEN DATAFILE";
	
	output.open(outputfile,ios::binary);
	if(output.is_open())
		cout<<"\nACYUT.DAT OPENED SUCCESSFULLY";
	else
		cout<<"\nUNABLE TO OPEN ACYUT.DAT";

	int records=0;
	while(1)
	{
		if(data.eof())
			break;
		data.read((char*)&entry[records],sizeof(entry[records]));
		++records;
	}

	data.close();

	parameters entry1= entry[0];
	parameters entry2;
	parameters missing_entry[20];

	for(int i=1;i<records;++i)
	{
		cout<<"\nkwjfk";
		entry2=entry[i];
		output.write((char*)&entry1,sizeof(entry1));
		int n=entry2.motor_pos - entry1.motor_pos -1;		//number of missing records
		if(n>=0)
		{
			float angle_diff=(entry2.angle - entry1.angle)/float(n+1);
			float focal_diff=(entry2.focal - entry1.focal)/float(n+1);
			float pix2cmx_diff=(entry2.pix2cmx - entry1.pix2cmx)/float(n+1);
			float pix2cmy_diff=(entry2.pix2cmy - entry1.pix2cmy)/float(n+1);
			float s_view_compensation_diff=(entry2.s_view_compensation - entry1.s_view_compensation)/float(n+1);

			for(int j=0;j<n;++j)
			{
				missing_entry[j].motor_pos=entry1.motor_pos+j+1;
				missing_entry[j].angle=entry1.angle+angle_diff*(j+1);
				missing_entry[j].focal=entry1.focal+focal_diff*(j+1);
				missing_entry[j].pix2cmx=entry1.pix2cmx+pix2cmx_diff*(j+1);
				missing_entry[j].pix2cmy=entry1.pix2cmy+pix2cmy_diff*(j+1);
				missing_entry[j].s_view_compensation=entry1.s_view_compensation+s_view_compensation_diff*(j+1);
			}

			for(int j=0;j<n;++j)
				output.write((char*)&missing_entry[j],sizeof(missing_entry[j]));
		}

		entry1=entry2;
	}

	output.close();
	// char x[50]="acyut";
	strcat(filename,"_interpolated");
	converttotext(filename);

	return 0;
}