/* NAME : PRAVEEN B R
DATE : 01/08/2024
DESCCRIPTION : LSB IMAGE STEGANOGRAPHY PROJECT 

 */


#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"
#include "common.h"
int main(int argc,char * argv[])
{
   
    //validate CLA
    if(argc > 2)
    {

        EncodeInfo E1;  //stricture to hold the encoding info
        int ret = check_operation_type (argv);  //variable to hold the operation type user has passed

        //if user has chosen to encode
        if(ret == e_encode)
        {
            //if the user has not passed the secret file show error
            if(! (argv[3]))
            {
               printf("error\n");
               printf("Usage : For Encoding : ./a.out -e beautiful.bmp secret.txt [stegano.bmp] \n");
               printf("     For Decoding : ./a.out -d stegano.bmp [decoded.txt] \n ");

            }

            //if all the required arguments are passed do encoding    
            else
            {
                printf("Info: Encoding is selected\n");
                //validate the passed files
    
                if(read_and_validate_encode_args(argv, &E1) == e_success)
                {
                    printf("Info : Read and validate args is success \n");
                    
                    //if required files are passed do encoding    
                    if(do_encoding(&E1) ==e_success)
                    {
                        printf("Encoding is success\n");
                    }
                    else
                    {
                        printf("failed to encode\n");
                        return 6;
                    }

                }
                else
                {
                    printf("Info : Read and validate args is failed\n");
                    return 5;
                } 
            }
        }

        //if usser has chosen to decode
        else if(ret == e_decode)
        {

            printf("Info: Decoding is selected\n");
    
            DecodeInfo d1;  //structure to hold the decoding info

            //to validate the passsed file
            if(read_and_validate_decode_args(argv,&d1) == e_success)
            {
                printf("Info: read and validate args is successful\n");
                
                //if required files are passed do decoding
                if(do_decoding(&d1) == e_success)
                {
                    printf("Decoding done Successfully\n");
                }
                else
                {
                    printf("Failed to  Decode\n");
                    return 5;
                }
            }
            else
            {
                printf("Read and validate args Failed\n");
                return 5;
            }
        }

        //if user has passed other than encode and decode
        else
        {
            printf("Error : Unsupported type \n" );
        }

    }

    //if required number of CLA not passed
    else
    {
        printf("error\n");
        printf("Usage : For Encoding : ./a.out -e beautiful.bmp secret.txt [stegano.bmp] \n");
        printf("     For Decoding : ./a.out -d stegano.bmp [decoded.txt] \n "); 
    }


    return 0;
}

//function to check the operation type passed by the user
OperationType check_operation_type(char *argv[])
{
    //if user has chosen to encode
    if(strcmp(argv[1] , "-e") == 0)
    {
        return e_encode;
    }
    //if used has chosen decode
    else if(strcmp(argv[1],"-d") == 0)
    {
        return e_decode;
    }
    //if user has passed an invalid argument
    else
    {
        return e_unsupported;
    }


}
