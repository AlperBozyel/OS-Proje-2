#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Include our own header files */
#include "parser.h"
#include "process.h"

#define VERSION "1.1"
#define PROMPT ">"

int main(int argc, char *argv[])
{
  int e;   // e degiskeni , pipe() fonksiyonunun donus degerini tutuyor.
  int reponse; // Chdir fonksiyonun donus degerini tutmak icin olusturdugumuz degisken.
  int status, status2, status3;  // child donus degerlerini tutmak icin degiskenleri
  int exit_code, exit_code2, exit_code3; // cocuk sürec donus degerleri
                             
  /* Komut satiri ile ilgili bilgileri tutan struct'i tanimliyoruz. */
  CommandLine cl;

  /* Yukaridaki cl degiskenini gosteren bir gosterici tanimliyoruz. */
  CommandLine *cl_ptr = &cl;

  /* TODO: Kullanicinin home dizinini kaydedin. */
  char *Home = getenv("/home");

  while (1)
  {
    /* Ekrana PROMPT ile tanimli komut satiri bekleyicisini yaziyoruz. */
    printf("%s ", PROMPT);

    /* shell_process_line() ile satiri okuyup ayristiriyoruz. Artik cl_ptr
         * ile gosterilen CommandLine yapisindaki komut satirina dair bilgiler
         * var.
         */
    shell_process_line(cl_ptr);

    /* Eger komut satiri okunurken bir hata olduysa, error_code degiskeni
         * ayarlaniyor. shell_print_error() ise struct'taki error_code'un
         * degerine bakarak hata varsa ekrana bunu basiyor. continue ile
         * dongunun basina donuyoruz.
         */
    if (cl_ptr->error_code > 0)
    {
      shell_print_error(cl_ptr);
      continue;
    }

    /* Boru hatti olsun olmasin, cl_ptr->first_argv icinde bir komut yaziyor
         * olmali. Eger NULL ise kullanici hicbir sey yazmadan Enter'a basip
         * gecti demektir.
         */
    if (cl_ptr->first_argv != NULL)
    {
      /* Once gomulu komut mu degil mi diye bakiyoruz. Eger
             * komut gomuluyse, boru hatti olup olmamasiyla ilgilenmiyoruz,
             * dogrudan calistiriyoruz.
             */
      if (strcmp(cl_ptr->first_argv[0], "version") == 0)
      {
        printf("Shell version: %s\n", VERSION);
        shell_free_args(cl_ptr);
      }

      else if (strcmp(cl_ptr->first_argv[0], "exit") == 0)
      {
        shell_free_args(cl_ptr);
        exit(EXIT_SUCCESS);
      }

      else if (strcmp(cl_ptr->first_argv[0], "cd") == 0)
      {
        /* TODO: chdir fonksiyonunu uygun sekilde calistirip donus degerini alin. Fonksiyon icine gidilecek
                dizin bilgisi yazilmaldir. Dizin bilgisi parametre olarak yok ise basta alinan ev dizinine gidilmelidir.*/

        if (cl_ptr->first_argv[1] == NULL)
        {
          chdir(Home);
        }
        else
        {
          reponse = chdir(cl_ptr->first_argv[1]);
        }

        /* TODO: Donus degeri 0'dan kucuk ise ekrana hata mesaji basalim. */
        if (reponse < 0)
        {
          perror("Gsu_Shell");
        }
        /* TODO: cl_ptr icin tutulan bellek alanini yukaridaki orneklerdeki gibi sisteme geri verelim */
        shell_free_args(cl_ptr);
      }

      /* Komut satiri gomulu komutla baslamiyorsa, normal
             * prosedur isleyecek: Komut(lar) yeni yaratilan cocuk
             * surec(ler) tarafindan exec edilecek.
             *
             * Ayrintilar icin PDF'e bakin!
             */
      else
      {
        /* Cocuk sureclerin PID'lerini tutmak icin. */
         pid_t first_child, second_child; 
         
        /* 2. Asamaya dair geri kalan her sey bu bolgeye yazilacak. 
		 * Burada tasarim daha cok sizden beklenmektedir.*/

        /* TODO: Ilk olarak pipe icin dosya tanimlayicilarini tutacak bir dizi tanimlayin.
		 * Eger komut satirinda pipe girilmis ise uygun sekilde boru hattini olusturun.
                 */
        
                 int pipe_fd[2];                 // pipe icin dosya tanimlayicilarini tutacak dizi

                 if(strcmp(cl_ptr->first_argv[1],"|") == 0) {
                 
                    e = pipe(pipe_fd);       
                    if(e == -1 ) {
                        perror("Can't Create Pipe!");
                        shell_free_args(cl_ptr);
                        exit(1);
                    } 
                                        
                 }   

               
        /* TODO: Bu noktada ise cocuk surec yaratmaya baslayacagiz. Eger pipe varsa parent surec iki 
		 * cocuk surec yaratmali ve cocuk surecler icinde dup2 fonksiyonu uygun sekilde kullanilmalidir. 
		 * Pipe yoksa da bir cocuk surec yaratilmalidir. Pipe olsa da olmasa da cl_ptr->first_argv degeri 
		 * birinci cocuk tarafindan shell_exec_cmd ile calistirilmalidir. 
		 * Pipe varsa cl_ptr->second_argv degeri ikinci cocuk  tarafindan calistirilmalidir. 
                 * Boru hattinin uclarinin kapatilmasi, cocuk surec veya sureclerin
                 * geri donus degerlerinin alinmasi ve verimli bellek kullanimi gibi islemler de uygulanmalidir.
 		 */
 		  
                 if( e == -1 ) {                        // boru hattı yoksa
 		  
 		      switch (first_child = fork()) {                          // 1.cocuk olusturuldu.
 		      
                        case -1:
                           perror("fork Error");
                           exit(1);
            
                        case 0:

                           exit_code = shell_exec_cmd( cl_ptr->first_argv );

                           exit(exit_code);
            
                        default:

                           /* Cocuk sureci bekleyelim. */
                           waitpid(first_child,&status,0);  // ilk cocuk bekleniyor.

                           exit_code = status;
                           shell_free_args(cl_ptr);
                           exit(0);
                    }
                 }
                 
                 if( e == 0 ) {                         //  boru hattı varsa

 		      switch (first_child = fork()) {                            // 1.cocuk olusturuldu.
 		      
                        case -1:
                        
                           perror("fork1 Error");
                           exit(1);
            
                        case 0: 
                                              
                           close(pipe_fd[0]);

                           exit_code2 = shell_exec_cmd( cl_ptr->first_argv );
                          
                           close(pipe_fd[1]);
                           exit(exit_code2);
            
                        default:
                           
                           /* İlk Cocuk sureci bekleyelim. */
                           waitpid(first_child,&status2,0);  // ilk cocuk bekleniyor.  
    
                           exit_code2 = status2;
                                                                            
                           switch (second_child = fork()) {                                // 2.cocuk olusturuldu.
                                                   
                           
                              case -1:
                                 perror("fork2 Error");
                                 exit(1);
                              
                           
                              case 0:                             
                                 exit_code3 = shell_exec_cmd( cl_ptr->second_argv );  
                                 exit(exit_code3);                                                          
                           
                              default:                                                      
                                  
                                 dup2(pipe_fd[1], STDOUT_FILENO);
                                 
                                 close(pipe_fd[1]);
                                                     
                                 waitpid(second_child,&status3,0);  // ilk cocuk bekleniyor.                                                           
                                 exit_code3 = status3;
                           
                                 close(pipe_fd[0]);
                                 shell_free_args(cl_ptr);                       
                                 exit(0);
                           }
                     }
                 } 	
                 	 


      } /* else */
    }   /* if (cl_ptr) */
  }     /* while(1) */

  return 0;
}
