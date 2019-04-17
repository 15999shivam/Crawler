#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstring>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
using namespace std;

typedef struct links
{
  char *link;
  links *next;
}links;

typedef struct hash2
{
  char *link;
  hash2 *next;
}hash2;

// Hash Function->returns hash value from 0-49
int hash1(char *str)
{
  //cout<<"hii";
  //cout<<str;
  int sum = 0;
  int i=0;
  while(str[i] != '\0')
  {
    sum+=str[i];
    i++;
  }
  return sum%50;
}
/*
*GetNextURL*
------------
Description: Given a HTML string buffer, the URL of the HTML,
and a position index, find the closest URL after the position
and copy the URL into the result buffer, which is also an input
argument. This function is the main component of the HTML parser.
This function is designed such that it is meant to be repeatedly 
called toextract URLs from the HTML one at a time, starting from the 
beginning of the HTML and terminating when the end of the HTML
is reached. The return value of this function is meant for the
repeated calls of this function; the real return value is the
third input argument, which is the result buffer, in which a
new URL will be written if one is found. This function can 
handle normal absolute and relative URLs generally  found in
the <a href=""> tags; however, more extreme cases, like this
<a href="../../../a.txt">, are not currently being extracted.
Input: html_buffer, urlofthispage, result_buffer, current_position
Return: Position of the URL found
** Pseudo Code **
(1) IF (first call) THEN
      Remove white space characters from the page
    END
(2) Find the <a> or <A> html tags by scanning through the html text
(3) Keep going until we may have found a URL
(4) IF (It actually is NOT a URL, which has multiple possibilities) THEN
      Recursively call self from the next position
    END
(5) IF (It is an absolute URL) THEN
      Set result buffer to contain this URL
      Return the current position
    ELSE (It is an relative URL) THEN
      Produce the result URL by combining the relative URL with the urlofthispage
      Set result buffer to contain this URL
      Return the current position
    END
(7) Return -1 to signal completion
*****
*/
void removeWhiteSpace(char* html);
int GetNextURL(char* html, char* urlofthispage, char* result, int pos) 
{
  char c;
  int len, i, j;
  char* p1;  //!< pointer pointed to the start of a new-founded URL.
  char* p2;  //!< pointer pointed to the end of a new-founded URL.

  // NEW
  // Clean up \n chars
  if(pos == 0) {
    removeWhiteSpace(html);
  }
  // /NEW

  // Find the <a> <A> HTML tag.
  while (0 != (c = html[pos])) 
  {
    if ((c=='<') &&
        ((html[pos+1] == 'a') || (html[pos+1] == 'A'))) {
      break;
    }
    pos++;
  }
  //! Find the URL it the HTML tag. They usually look like <a href="www.abc.com">
  //! We try to find the quote mark in order to find the URL inside the quote mark.
  if (c) 
  {  
    // check for equals first... some HTML tags don't have quotes...or use single quotes instead
    p1 = strchr(&(html[pos+1]), '=');
    
    if ((!p1) || (*(p1-1) == 'e') || ((p1 - html - pos) > 10)) 
    {
      // keep going...
      return GetNextURL(html,urlofthispage,result,pos+1);
    }
    if (*(p1+1) == '\"' || *(p1+1) == '\'')
      p1++;

    p1++;    

    p2 = strpbrk(p1, "\'\">");
    if (!p2) 
    {
      // keep going...
      return GetNextURL(html,urlofthispage,result,pos+1);
    }
    if (*p1 == '#') 
    { // Why bother returning anything here....recursively keep going...

      return GetNextURL(html,urlofthispage,result,pos+1);
    }
    if (!strncmp(p1, "mailto:",7))
      return GetNextURL(html, urlofthispage, result, pos+1);
    if (!strncmp(p1, "http", 4) || !strncmp(p1, "HTTP", 4)) 
    {
      //! Nice! The URL we found is in absolute path.
      strncpy(result, p1, (p2-p1));
      return  (int)(p2 - html + 1);
    } else {
      //! We find a URL. HTML is a terrible standard. So there are many ways to present a URL.
      if (p1[0] == '.') {
        //! Some URLs are like <a href="../../../a.txt"> I cannot handle this. 
	// again...probably good to recursively keep going..
	// NEW
        
        return GetNextURL(html,urlofthispage,result,pos+1);
	// /NEW
      }
      if (p1[0] == '/') {
        //! this means the URL is the absolute path
        for (i = 7; i < strlen(urlofthispage); i++)
          if (urlofthispage[i] == '/')
            break;
        strcpy(result, urlofthispage);
        result[i] = 0;
        strncat(result, p1, (p2 - p1));
        return (int)(p2 - html + 1);        
      } else {
        //! the URL is a absolute path.
        len = strlen(urlofthispage);
        for (i = (len - 1); i >= 0; i--)
          if (urlofthispage[i] == '/')
            break;
        for (j = (len - 1); j >= 0; j--)
          if (urlofthispage[j] == '.')
              break;
        if (i == (len -1)) {
          //! urlofthis page is like http://www.abc.com/
            strcpy(result, urlofthispage);
            result[i + 1] = 0;
            strncat(result, p1, p2 - p1);
            return (int)(p2 - html + 1);
        }
        if ((i <= 6)||(i > j)) {
          //! urlofthis page is like http://www.abc.com/~xyz
          //! or http://www.abc.com
          strcpy(result, urlofthispage);
          result[len] = '/';
          strncat(result, p1, p2 - p1);
          return (int)(p2 - html + 1);
        }
        strcpy(result, urlofthispage);
        result[i + 1] = 0;
        strncat(result, p1, p2 - p1);
        return (int)(p2 - html + 1);
      }
    }
  }    
  return -1;
}

/*
*NormalizeWord*
---------------
Description: Make sure all the Roman letters in the URL are
of lower cases, for ease of carrying out string comparison in
the future when trying to decide if two URL are the same or not.
Basically a linear scan, starting from the beginning of the URL,
is performed. Whenever a capital letter character is encountered
(by checking its ASCII code value), it is replaced by the
corresponding lower case letter.
Input: input_url
** Pseudo Code **
(1) FOR (every character in the input string) DO
      IF (this character is a capital letter) DO
        Change this letter to lower case
      END
    DONE
*****
*/

void NormalizeWord(char* word) {
  int i = 0;
  while (word[i]) {
      // NEW
    if (word[i] < 91 && word[i] > 64) // Bounded below so this funct. can run on all urls
      // /NEW
      word[i] += 32;
    i++;
  }
}

/*
*NormalizeURL*
--------------
Description: Normalize the input URL string and return its validity.
The normalization contains two major components: first, if the
URL ends with a trailing slash '/' character, this trailing slash
will be removed from the URL; and second, if the URL points to
a file (with an extension), then only certain file extensions are
accepted; currently, acceptable normal file extensions start with
.htm, .HTM, .php, or .jsp. A URL is valid if it is long enough and,
if pointing to a file, points to a file of acceptable format;
otherwise the URL is considered invalid and therefore will not
be added to the url_list for future processing.
Input: input_url
Output: validity of the input URL: 0 - invalid
                                   1 - valid
** Pseudo Code **
(1) Return error signal if input url is too short.
(2) IF (input url ends with '/') THEN
      Remove the '/'
    END
(3) Find the positions of the last occurrences of '/' and '.'
(4) IF (the '/' and '.' are positioned s.t. they indicate the url points to a file) THEN
      IF (the file extension starts with .htm or .HTM or .php or .jsp) THEN
        Do nothing...
      ELSE
        Return bad url signal
      END
    END
(5) Return good url signal
*****
*/

int NormalizeURL(char* URL) 
{
  int len = strlen(URL);
  if (len <= 1 )
    return 0;
  //! Normalize all URLs.
  if (URL[len - 1] == '/') 
  {
    URL[len - 1] = 0;
    len--;
  }
  int i, j;
  len = strlen(URL);
  //! Safe check.
  if (len < 2)
    return 0;
  //! Locate the URL's suffix.
  for (i = len - 1; i >= 0; i--)
    if (URL[i] =='.')
      break;
  for (j = len - 1; j >= 0; j--)
    if (URL[j] =='/')
      break;
  //! We ignore other file types.
  //! So if a URL link is to a file that are not in the file type of the following
  //! one of four, then we will discard this URL, and it will not be in the URL list.
  if ((j >= 7) && (i > j) && ((i + 2) < len)) 
  {
    if ((!strncmp((URL + i), ".htm", 4))
        ||(!strncmp((URL + i), ".HTM", 4))
        ||(!strncmp((URL + i), ".php", 4))
        ||(!strncmp((URL + i), ".jsp", 4))
        ) 
    {
      len = len; // do nothing.
    } 
    else 
    {
      return 0; // bad type
    }
  }
  return 1;
}

/*
*removeWhiteSpace*
------------------
Description: Removes the white space characters from the input
string buffer that contains the html content. This function
basically scans through the entire html buffer content character
by character, and abandons any white space character it encounters.
The ASCII code of the characters are used to determine whether
a character is a white space or not; Characters with ASCII code
values below 32 are considered white space characters, and are
thus removed.
Input: string_buffer
** Pseudo Code **
(1) Create a target buffer one character than the input buffer, and clear it
(2) FOR (Every character in the input buffer) DO
      IF (the current character is not a while space character) THEN
        Append it to the end of the target buffer
      END
    DONE
(3) Overwrite the input buffer with the target buffer
(4) Release targer buffer
    
*****
*/

void removeWhiteSpace(char* html) 
{
  int i;
  char *buffer = (char *)malloc(strlen(html)+1), *p=(char *)malloc (sizeof(char)+1);
  memset(buffer,0,strlen(html)+1);
  for (i=0;html[i];i++) 
  {
    if(html[i]>32)
    {
      sprintf(p,"%c",html[i]);
      strcat(buffer,p);
    }
  }
  strcpy(html,buffer);
  free(buffer); free(p);
}
void getPage(char *link,char *file)
{
   char lnk[100]="wget ";
        strcat(lnk,link);
        system(lnk);
        char dir[100]="mv index.html ";
        strcat(dir,file);
        
        //cout<<"*"<<dir<<"*"<<endl;
        
        system(dir);
       // cout<<"hiiii";
        ofstream fout(file,ios::app);
        if(!fout)
        {
         // cout<<"file not open\n";
        }
        else
        {
         //  cout<<"opened";
        fout<<link;
        fout.close();
        }
}
int main(int argc,char *argv[])
{
    if(argc<4)
    {
    printf("Please give link depth filename.txt\nerr:too few arguments\n");
    }
    else
    {
        int depth=atoi(argv[2]);
        getPage(argv[1],argv[3]);
        /*char lnk[100]="wget ";
        strcat(lnk,argv[1]);
        system(lnk);
        char dir[100]="mv index.html ";
        strcat(dir,argv[3]);
        
        //cout<<"*"<<dir<<"*"<<endl;
        
        system(dir);
       // cout<<"hiiii";
        ofstream fout(argv[3],ios::app);
        if(!fout)
        {
         // cout<<"file not open\n";
        }
        else
        {
         //  cout<<"opened";
        fout<<argv[1];
        fout.close();
        }
       */
        
    }
   // int file_no = 1;
   struct stat st; //variable which will count length of file.
   char file1[30] = "/home/shivam/Desktop/crawler/";
   strcat(file1,argv[3]);
   //cout<<file1;
   
stat(file1,&st); // temp.txt is the file where wget fetch the html
int file_size=st.st_size;
cout<<file_size<<endl;

char *file = (char *)malloc(file_size*2);
//cout<<file_size;
char *ptr;
ifstream fin(argv[3]);
int it=0;
char ch;
while(!fin.eof())
{
 ch = fin.get();
 file[it]=ch;
 it++;
}
//cout<<file;

char *urls = (char*)malloc(100*sizeof(char));
 int n = GetNextURL(file, argv[1], urls, 0);
 links *head = (links *) malloc(sizeof(links));
links *temp = head; 
char *links_arr[100];
 for(int i=0;i<100;i++)//linked list and links_array are generated side by side
 {
   //cout<<urls<<endl;
   int flag = 0;
   for(int j = 0;j<i;j++) //check if link previously link exists
   {
     if(!(strcmp(links_arr[j],urls)))
     {
       flag = 1;
     }
   }
  if(flag==1) //fuck of this itration if found url...means do not consider this iteration
  {
    n = GetNextURL(file, argv[1], urls, n+1);//go for next url
    i--;
    continue;
  }

   links_arr[i] = urls;
   temp->link = urls;
   // cout<<"*"<<strcmp(links_arr[0],urls)<<"*"<<endl;
   urls = (char*)malloc(100*sizeof(char));

   n = GetNextURL(file, argv[1], urls, n+1);
   if(i>98 || n==-1 )
   {
     temp->next=NULL;
     break;
   }
   links *second = (links *) malloc(sizeof(links));
  temp->next = second;
  temp= temp->next;
   //cout<<urls<<endl;
 }
 temp = head;
 int i=0;/*
while(temp!=NULL)
{
 cout<<temp->link<<"   "<<i<<endl;
temp= temp->next; 
i++;
}*/
/*cout<<argv[3]<<" "<<argv[1]<<endl;
cout<<"*******"<<urls<<"**********";

*/
/*sprintf(itostring,"%d",file_no); //converts integer into string
char file[]="/temp"; //name of file.
char f[]=".txt";
strcat(p,path); //creating path using strcat in p.
strcat(p,file);
strcat(p,itostring);
strcat(p,f);
printf("PATH has been created\n");*/

hash2 *link_hash[50];
for(i=0;i<50;i++)
{
  link_hash[i] = NULL;
}
for( i=0;i<100;i++)
{
  //cout<<links_arr[i]<<endl;
 //cout<<"hiii"<<endl;
  int index = hash1(links_arr[i]);
  hash2 *tempp;
  tempp = link_hash[index];
  hash2 **prev;
  prev = &link_hash[index];
  //cout<<"prev = "<<prev<<endl;
  //cout<<"*****"<<(temp!=NULL)<<"*****"<<endl;
  while(tempp!=NULL)
  {
    prev = &(tempp->next);
    tempp = tempp->next;
  }
  if(index==4){
    cout<<links_arr[i]<<endl;
  }
   tempp = (hash2*)malloc(sizeof(hash2));
    tempp->link = (char*)malloc(sizeof(char)*110);
   strcpy(tempp->link,links_arr[i]);
  // cout<<tempp->link<<" "<<i<<endl;
  
  // tempp->link = char1;
  // //cout<<temp->link<<"*"<<endl;
   tempp->next = NULL;
  if(*prev == NULL)
  {
    cout<<"ye purana hai"<<endl;
    *prev = tempp;
  }
  else
  {
    cout<<"ye nya hai"<<endl;
    (*prev)->next = tempp;
  }
   
}
 hash2 *temp12;
  temp12 = link_hash[4];
 
  while(temp12!=NULL)
  {
    cout<<"hii";
    cout<<temp12->link<<endl;
    temp12 = temp12->next;
  }
}