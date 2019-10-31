#include <stdio.h>
#include <string.h>

int main()
{

    printf("Here\n");

    
    char str1[1024] = "CAPS LOCK - Preventing Login Since 1980.\n"
                 "Computers make very fast, very accurate mistakes.\n"
                 "My software never has bugs. It just develops random features.\n"
                 "\n";
                 
    char str2[1024] = "I would love to change the world, but they won't give me the source code.\n"
                 "Hey! It compiles! Ship it!\n"
                 "The more I C, the less I see.\n"
                 "My attitude isn't bad. It's in beta.\n";

    strcat(str1, str2);

    int line_no = 0;

    char* token = strdup("");

    while ((token = strsep(*str1, "\n")) != NULL)
    {
        sprintf(str1 + strlen(str1), "%d %s", line_no++, token);
        //strcat(str, temp);
    }

    printf("%s", str1);
    

    return 0;
}