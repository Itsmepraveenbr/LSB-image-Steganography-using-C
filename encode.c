#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include  "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

//to get the size of rgb data by multiplying height with width x 3(rgb)
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte to read the width 
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width of image = %u\n", width);

    // Read the height (an int) next 4bytes of info
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height of image = %u\n", height);

    // Return image capacity(height * width * (RGB bytes))
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

//opening the files passed by the user
Status open_files(EncodeInfo *encInfo)
{
    printf("Opening required Files\n");

    // Src Image file
    encInfo -> fptr_src_image = fopen(encInfo -> src_image_fname, "r");
    // Do Error handling
    if (encInfo -> fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo -> src_image_fname);

    	return e_failure;
    }
    else
    {
        printf("INFO: opened %s file\n",encInfo -> src_image_fname);
    }

    // Secret file
    encInfo -> fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo -> fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo -> secret_fname);

    	return e_failure;
    }
    else
    {        
        printf("INFO: opened %s file\n",encInfo -> secret_fname);
    }

    // Stego Image file in write mode
    encInfo -> fptr_stego_image = fopen(encInfo -> stego_image_fname, "w");
    // Do Error handling
    if (encInfo -> fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo -> stego_image_fname);

    	return e_failure;
    }
    else
    {
        printf("INFO: opened %s file\n",encInfo -> stego_image_fname);

    }

    // No failure return e_success
    return e_success;
}

//to read and validate the passed files
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //to check if user has passed a .bmp file 
   if(strcmp( strstr(argv[2], ".") ,".bmp") == 0)
   {
       encInfo -> src_image_fname = argv[2];   //storing the .bmp file name in one variable
   }
   else
   {
      return e_failure;
   }

   //( strcmp( strstr( argv[3],".") , ".txt") == 0)
  // {
        //storing the secret file to be encoded in one variable
        encInfo -> secret_fname = argv[3];  //storing file of any extension 
  /* }
   else
   {
       return e_failure;
   }*/

   //if user has not passed the image to hold the encoding info
   if(argv[4] != NULL)
   {
      encInfo -> stego_image_fname = argv[4];
     
   }
   //if user did not file the image to hold the encoded info create an image file
   else
   {
         printf("Info: output file not mentioned, creating default.bmp as default\n");
        encInfo -> stego_image_fname = "default.bmp";
   }
 return e_success;

}

//to get the size of file
uint get_file_size(FILE *fptr)
{
    fseek(fptr ,0 ,SEEK_END);  //to move the pointer till the end of file

    return  ftell(fptr);    //return the position of the file pointer

}

//to check capacity of image and the information to be encoded if the encoded info can fit in the image or not
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image); //finding size of RGB data in bytes
    encInfo -> size_secret_file =  get_file_size(encInfo -> fptr_secret);//finding size of secret file size

    //to encode 1 byte of data 8 bytes are needed 
    //checking if the image size is more and we can encode the secret info in it or not (1. magic string(2 bytes *8), 2. password length (4*8) 3. pasword(password size * 8) 4. secret file extension size(4*8), 5.secret file extension (4*8), 6.secret file size (4*8), 7. secret data (secret_file_size * 8)
    
    if((encInfo -> image_capacity) > ( 16 + 32 + (strlen(password) * 8) + 32 + 32 + 32 + 32 + encInfo->size_secret_file * 8))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
            
}

//to copy the 54 bytes of header to the stegno image file
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char str[54];

    //bring the file offset to starting position
    fseek(fptr_src_image, 0, SEEK_SET);

    //read 54 bytes from beautiful.bmp file and copy it to the stegno image
    fread(str, 54, 1, fptr_src_image);
    fwrite(str,54,1, fptr_dest_image);

    return e_success;
}

//to encode 1 byte of secret data in 8 bytes of image data
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    //encode byte to lsb
    for(int i = 0 ; i < 8 ; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> i) &1);
    }
    //result stored in same array image_buffer
    return e_success;
}

//to encode the secret info into the image datza
Status encode_data_to_image(char *data, int size, EncodeInfo *encInfo)
{
    //to loop for size number of times and encoding 1 byte of secret with 8 bytes of image data
    for(int i = 0 ; i < size ; i++)
    {
        fread(encInfo -> image_data, 8,1, encInfo -> fptr_src_image );
        encode_byte_to_lsb(data[i], encInfo -> image_data);
        fwrite(encInfo -> image_data, 8, 1, encInfo -> fptr_stego_image);    

    }
        return e_success;

}
//to encode the size data into image data(4 bytes of secret data into 32 bytes of image data)
Status encode_size_to_lsb( int size, EncodeInfo *encInfo)
{
    char str[32];
    fread(str, 32, 1, encInfo -> fptr_src_image);
    
    //to loop for 32 times by encding every bit of secret data in lsb of every bite of image data
    for(int i = 0 ; i < 32 ; i++)
    {
         str[i] = (str[i] & 0xFE) | (size >> i & 1);
        
    }
     
    fwrite(str, 32, 1, encInfo -> fptr_stego_image);
    return e_success;
}

//to rncode the secret file extension size data into image data
 Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
   
    encode_size_to_lsb(size, encInfo);
    return e_success;
}

Status encode_magic_string( char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image(magic_string, strlen(magic_string), encInfo);
    return e_success;
}

Status encode_password_length(int password_length ,EncodeInfo *encInfo)
{
  
    encode_size_to_lsb(password_length, encInfo);
    return e_success;
}


//to encode the magic string / password into the image data
Status encode_password( char *password, EncodeInfo *encInfo)
{
  
    encode_data_to_image(password, strlen(password), encInfo);
    return e_success;

}

//to encode the extension of secret file into image data
Status encode_secret_file_extn( char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image(file_extn, strlen(file_extn), encInfo);
    return e_success;

}

//to encode the secret file size into the image data
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    encode_size_to_lsb(file_size, encInfo);
    return e_success;

}

//to encode the secret data into the image data
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    //array of the size of secret file to hold the secret data
    char str[encInfo -> size_secret_file]; 

    //to move the file offset to the starting
    fseek(encInfo -> fptr_secret, 0, SEEK_SET);  

    //to read the secret file data into the array
    fread(str, encInfo -> size_secret_file, 1, encInfo -> fptr_secret); 

    //to write the array data into the steno image by calling encode data to image function
     encode_data_to_image(str, encInfo -> size_secret_file, encInfo); 
    return e_success;
}

//to copy the remaining source image data into the stegno image 
Status copy_remaining_img_data(EncodeInfo *encInfo)
{
    //to calculate the length of data left in th source image that has to be copied by( 54 bytes of header + RGB data - the size of data until where we have already encoded)
    int length = 54 + (encInfo -> image_capacity) -ftell(encInfo -> fptr_stego_image); 

    char str[length]; //array to hold the remaining data

    fread(str, length, 1,encInfo -> fptr_src_image);  // reading the data from source image into array
    fwrite(str, length, 1, encInfo -> fptr_stego_image); //writing teh data from array to source image
           return e_success;

}


//to do encoding step by step by calling required functions
Status do_encoding(EncodeInfo *encInfo)
{
   if( open_files(encInfo) == e_success)
   {
       printf("Open files is success \n");

       //to ask user to create a password to encode
       printf("Create a password to encode \n");
       scanf("%[^\n]", password);
        
       //display the password entered by the user
       printf("The password you entered is : %s \n", password);
       printf("\n");
       if(check_capacity(encInfo) == e_success )
       {
        printf("Info: Check capacity is success\n");

            if( copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
            {
                printf("Info: Copied bmp header successfully\n");

                if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    
                    printf("Info : Encoded magic string successfullly\n");
                
                    if(encode_password_length(strlen(password),encInfo) ==e_success)
                    {
                        printf("Info: Encoded password length successfully\n");

                        if(encode_password(password, encInfo) == e_success)
                        {
                            printf("Info: Encoded password successfully\n");
                            strcpy(encInfo -> extn_secret_file , (strstr(encInfo -> secret_fname, ".")));
                            //printf("%s\n",encInfo->extn_secret_file);
               
                            if(encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file), encInfo ) == e_success)
                            {
                                printf("Info: Encode secret file extn size is successful\n");
                    
                                if(encode_secret_file_extn(encInfo -> extn_secret_file, encInfo) == e_success)
                                {
                                    printf("Info: Encodecoded secret file extension successful\n");
                        
                                    if(encode_secret_file_size(encInfo -> size_secret_file,encInfo) == e_success)
                                    {
                                        printf("Info: Encoded secret file size successfully\n");
                            
                                        if(encode_secret_file_data(encInfo) == e_success)
                                        {
                                            printf("Info: Encoded secret file data successfully\n");
                                
                                            if(copy_remaining_img_data(encInfo) == e_success)
                                            {
                                                printf("Info: Copied remaining data successfully\n");
                                            }
                                            else
                                            {
                                                printf("Info: Failed to copy remaining data\n");
                                                return e_failure;
                                            }
                                        }
                                        else
                                        {
                                            printf("Info: Failed to encode secret data \n");
                                             return e_failure;
                                        }
                                    }
                                    else
                                    {
                                        printf("Info: Failed to encode secret file size\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Info: Failed to encode secret file extn\n");
                                    return e_failure;
                                }

                            }
                            else
                            {
                                printf("Info: Failed to encode secret file extn size\n");
                                return e_failure;
                            }
                        }
                        else
                        {

                            printf("Info: Failed to encode password\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Info: Failed to encode password length\n");
                    }
                }
                else
                {
                    printf("Info: Failed to encode Magic string\n");
                }

                }
                else
                {
                     printf("Info: Failed to copy bmp header\n");
                        return e_failure;
                }
            } 
            else
            {
                printf("Info: Secret data cannot fit in original image\n");
            }
        }

        else
        {
            printf("Info: Failed to open files\n");
            return e_failure;
        }
   return e_success;
}



