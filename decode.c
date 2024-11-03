#include<stdio.h>
#include "decode.h"
#include "types.h"
#include<string.h>
#include "encode.h"
#include "common.h"
#include<stdlib.h>


/* Read and validate decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decoInfo)
{
    //check if the  .bmp image file is passed 
    if(argv[2] != NULL)
    {
        if(strcmp(strstr(argv[2], "."), ".bmp") == 0)
        {
            decoInfo -> stego_image = argv[2];
        }
        else
        {   
            printf("pass a valid file\n");
            return e_failure;
        }
        //check if the file where decoded info is passed or not if not passed make a newfile
        if(argv[3] == NULL)                                                	
	    {
            printf("INFO : The output decoded data file is not passed , so created decode.txt\n");
	        decoInfo -> decode_file = "decoded_Info";                           	   
	    }
	    else                                                              	
	    {
	        decoInfo -> decode_file = argv[3];                               		
        }
	    return e_success;
    }
    else
    {
	    printf("INFO : Error in passing command line arguments\n\tUsuage: ./a.out -d stego.bmp\n");   //print error message if CLA is not passed properly
	    return e_failure;
    }
}

//to open the passed file
Status Open_files(DecodeInfo *decInfo)
{
    decInfo -> fptr_stego_image = fopen(decInfo -> stego_image, "r");          //open a file in read mode for a encoded data file
    /* error handling */
    if(decInfo -> fptr_stego_image == NULL)                                         //fptr_stego_image is NULL
    {
	    perror("fopen");
	    fprintf(stderr, "Error: Unable to open file %s\n", decInfo -> stego_image);           //print error message
	    return e_failure;
    }

    return e_success;
}



/* Function defination to decode byte */
Status decode_byte_from_lsb(char *data)                                          
{
    *data = (*data) & 1;       //perform a encoded data with bitwise AND operation to get th last bit
    return e_success;
}

/* Function defination for decode magic string */ 
Status decode_magic_string(DecodeInfo *decInfo)
{
    char magic_char = 0, buffer[MAX_IMAGE_BUF_SIZE];   //initialization of vaiable
    char magic_str[strlen(MAGIC_STRING) + 1];   //array to store the magic string read from the file
   
    //to skip the header part 
    fseek(decInfo -> fptr_stego_image, 54L, SEEK_SET);                    		 

    for(int i = 0; i < strlen(MAGIC_STRING) ; i++)    //run a loop till the length of MAGIC_STRING content
    {
	    magic_char = 0;
	    fread(&buffer, sizeof(char), 8, decInfo -> fptr_stego_image);        //read a contents from MAGIC_STRING and store it in a pointer
       	
        for(int j = 0 ; j < 8 ; j++)    //run a loop to read a 1byte of character
	    {
	          decode_byte_from_lsb(&buffer[j]);            //function call for decode byte from lsb to read a lsb bit
	          magic_char = (buffer[j] << j) | magic_char;     //store in a variable
	    }

	    magic_str[i] = magic_char;             //store a first character data
	    magic_char = 0;                       //make magic char to 0 to read and decode a next character
    }

    magic_str[strlen(MAGIC_STRING)] = '\0'; //make a length of MAGIC STRING to null

    if(strcmp(magic_str, MAGIC_STRING) == 0)     //compare whether a decoded magic string data and original magic string  are equal or not
    {
	    printf("INFO : Magic strings are equal, data is decoded from the image\n");        //print a message  both magic string are equal 
    }
    else
    {
	    printf("ERROR : Magic strings are not equal, data is not decoded from  the image\n");      	//print error message if decoded magic string and original magic string
	    return e_failure;
    }
    return e_success; 
}

//to decode the password
Status decode_password(const char * dec_password, DecodeInfo *decInfo)
{
     char magic_char = 0, buffer[MAX_IMAGE_BUF_SIZE];  //initialization of vaiable
     char magic_str[(decInfo -> password_size) + 1];  //to store the password obtained from the file                          
  
     for(int i = 0; i < ((decInfo->password_size)) ; i++)   //run a loop till the length of the password
     {
            magic_char = 0;
            fread(&buffer, sizeof(char), 8, decInfo -> fptr_stego_image);  //read a contents of password  and store it in a pointer
       
            for(int j = 0 ; j < 8 ; j++)      //run a loop to read a 1byte of character
            {
                decode_byte_from_lsb(&buffer[j]);    //function call for decode byte from lsb to read a lsb bit
                magic_char = (buffer[j] << j) | magic_char;    //store in a variable
            }
 
            magic_str[i] = magic_char;          //store a first character data    
            magic_char = 0;                 //make magic char to 0 to read and decode a next character
     }
 
     magic_str[decInfo->password_size] = '\0';   //make a length of password to null

     //compare whether a decoded password and original password are equal or not                                       
     if(strcmp(magic_str, dec_password) == 0) 
     {
         printf("INFO : Password matching\n");   //print a message  both passwords  are equal 
     }
     else
     {
         printf("ERROR : Password MISMATCH !!\n");  //print error message if decoded password and original password not equal
         return e_failure;
     }

     return e_success; 
 }


//to decode the password length
Status decode_password_length(DecodeInfo *  decInfo)
{
     char buffer[32];     //variable to hold the int integer value
     int password_size = 0;

     fread(&buffer,1, 32, decInfo ->fptr_stego_image);  //read a content from file and store it in a pointer
     
     for(int i = 0 ; i < 32 ; i++)
     {        
         decode_byte_from_lsb(&buffer[i]);
         password_size = ((int)buffer[i] << i) | password_size;
         
     }

     decInfo -> password_size = password_size;   //store a size of password into a structure variable

     return e_success;

}


/* Function defination to decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];        //initialize buffer with size of 32 bits because of storing of integer value
    int extn_size = 0;

    //read a size from a file and store it in a buffer
    fread(&buffer, sizeof(char), 32, decInfo -> fptr_stego_image);                                  
    
    for(int i = 0 ; i < 32 ; i++)       //run a loop to read a integer value
    {
	    decode_byte_from_lsb(&buffer[i]);      //function call to get a lsb bit till 32 bytes
	    extn_size = ((int)buffer[i] << i) | extn_size;   //store a value of decoded size into a variable
    }
    
    //call the functiion to decode the secret file extension
    if(decode_secret_file_extn(decInfo, extn_size) == e_success)          
    {
	    printf("INFO : Secret file extension is been decoded successfully and attached extension to the file\n");
    }
    return e_success;
}

/* Function defination for decode secrete file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo, int extn_size)
{
    char buffer[MAX_IMAGE_BUF_SIZE], file_extn[extn_size], extn_char = 0;  //declaration of variable 
    
    for(int i = 0 ; i < extn_size ; i++)  //run a loop till a extension size to get the extension name
    {
	    fread(&buffer, sizeof(char), MAX_IMAGE_BUF_SIZE, decInfo -> fptr_stego_image); //read a extension from file and store it in a pointer
	    
        for(int j = 0 ; j < MAX_IMAGE_BUF_SIZE ; j++)  //run a loop to 8 times to read a character
	    {
	        decode_byte_from_lsb(&buffer[j]);    //fetch a encoded data from  lsb 
	        extn_char = (buffer[j] << j) | extn_char;  //store a data in a variable
	    }

	    file_extn[i] = extn_char;    //store characters in a variable
	    extn_char = 0;          //make extn_char as 0 to read a next character data
    }

    file_extn[extn_size] = '\0';  

    strcpy(decInfo -> decode_file_name, decInfo -> decode_file);  //copy data from decode file to decodefile name
    strcat(decInfo -> decode_file_name, file_extn);   //cancatenate  a file extension to a decode file name
    
    decInfo -> fptr_decode_file = fopen(decInfo -> decode_file_name, "w"); //open a file in write mode to store a decoded data 

    if(decInfo -> fptr_decode_file == NULL)   //check whether decode file is null 
    {
	    perror("fopen");       //print a error message 
	    fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo -> decode_file_name);
	    return e_failure;
    }
    return e_success;
}


/* Function defination to decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];                                                   
    int secret_file_size = 0;
    fread(&buffer,1, 32, decInfo -> fptr_stego_image);  //read a content from file and store it in a pointer
   
    for(int i = 0 ; i < 32 ; i++)                                                                                                                                          
    {
	    decode_byte_from_lsb(&buffer[i]);
	    secret_file_size = ((int)buffer[i] << i) | secret_file_size;
    }

    decInfo -> secret_file_size = secret_file_size;  //store a size of file content in a structure element
    return e_success;
}

/* Function defination to decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[MAX_IMAGE_BUF_SIZE], file_data = 0;

    for(int i = 0 ; i < decInfo -> secret_file_size ; i++)     //run a loop till a file size to copy a content to file
    {
        fread(&buffer,1, 8, decInfo -> fptr_stego_image);   //read a content from a file and store it in a pointer 
        
        for(int j = 0 ; j < 8; j++)     //run a loop to 8 times to decode a single character
	    {
	        decode_byte_from_lsb(&buffer[j]);        //function call
            file_data = (buffer[j] << j) | file_data;   //store a data in a variable
        }

        fwrite(&file_data,1, 1, decInfo -> fptr_decode_file); //write a stored data into a file 
        file_data = 0;           //make file data to 0 to read a next character
    }

    return e_success;
}

//to decode the image file
Status do_decoding(DecodeInfo *decInfo)
{

      if(Open_files(decInfo) == e_success)         						
      {
	     printf("INFO : Opening files are succesful\n");

         if(decode_magic_string(decInfo) == e_success)
         { 
             printf("Info :Decode magic string success\n");
    
             if(decode_password_length(decInfo) == e_success)
             {
                    char dec_password[50]; //variable to store the user entered password
                
                    //ask user to enter the password
                    printf("Enter the password to proceed \n");
                    scanf("%[^\n]", dec_password);
            
                    if(decode_password(dec_password, decInfo) == e_success)
                    {
                        printf("Info : Entered Passsword is correct, proceeding with further steps\n");

                        if(decode_secret_file_extn_size(decInfo) == e_success)       
                        {
	                        printf("INFO : Decoding of file extension is successful\n");

                            if(decode_secret_file_size(decInfo ) == e_success)              
                            {
	                            printf("INFO :Decoding of secrete file size is successful\n");

                                if(decode_secret_file_data(decInfo) == e_success)                                        
                                {
	                                printf("INFO : Decoding of secrete file data is successful\n");
                                }
                                else
                                {
	                                printf("Error : in decoding of secrete file data\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
	                            printf("Error : in decoding of secrete file\n");
                                return e_failure;
                            }
                        }
                        else
                        {
	                        printf("Error : in encoding of file extension\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Please enter a valid password !!\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("unable to decode password length !!\n");
                    return e_failure;
                }
            }
            else
            {
                 printf("Faied to decode magic string\n");
                 return e_failure;
            }            
        }
        else
        {
             printf("Error in opening files\n");
             return e_failure;
        }

      return e_success;
}

