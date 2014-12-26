/* Encrypt data. The data is in the source code itself. */

#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <gpgme.h>

#define fail_if_err(err)					\
  do								\
    {								\
      if (err)							\
        {							\
          fprintf (stderr, "%s:%d: %s: %s\n",			\
                   __FILE__, __LINE__, gpgme_strsource (err),	\
		   gpgme_strerror (err));			\
          exit (1);						\
        }							\
    }								\
  while (0)

#define KEYRING_DIR "/var/tmp/mycrypto"

#define SENTENCE "I swear it is true"

#define MAXLEN 4096

int main(int argc, char **argv) {
  gpgme_error_t error;
  gpgme_engine_info_t info;
  gpgme_ctx_t context;
  gpgme_key_t recipients[2] = {NULL, NULL};
  gpgme_data_t clear_text, encrypted_text;
  gpgme_encrypt_result_t  result;
  gpgme_user_id_t user;
  char *buffer;
  ssize_t nbytes;

  /* Initializes gpgme */
  gpgme_check_version (NULL);
  
  /* Initialize the locale environment.  */
  setlocale (LC_ALL, "");
  gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
#ifdef LC_MESSAGES
  gpgme_set_locale (NULL, LC_MESSAGES, setlocale (LC_MESSAGES, NULL));
#endif

  error = gpgme_new(&context);
  fail_if_err(error);
  /* Setting the output type must be done at the beginning */
  gpgme_set_armor(context, 1);

  /* Check OpenPGP */
  error = gpgme_engine_check_version(GPGME_PROTOCOL_OpenPGP);
  fail_if_err(error);
  error = gpgme_get_engine_info (&info);
  fail_if_err(error);
  while (info && info->protocol != gpgme_get_protocol (context)) {
    info = info->next;
  }  
  /* TODO: we should test there *is* a suitable protocol */
  fprintf (stderr, "Engine OpenPGP %s is installed at %s\n", info->version,
	   info->file_name); /* And not "path" as the documentation says */

  /* Initializes the context */
  error = gpgme_ctx_set_engine_info (context, GPGME_PROTOCOL_OpenPGP, NULL,
				     KEYRING_DIR);
  fail_if_err(error);

  error = gpgme_op_keylist_start(context, "John Smith", 1);
  fail_if_err(error);
  error = gpgme_op_keylist_next(context, &recipients[0]);
  fail_if_err(error);
  error = gpgme_op_keylist_end(context);
  fail_if_err(error);

  user = recipients[0]->uids;
  printf("Encrypting for %s <%s>\n", user->name, user->email);

  /* Prepare the data buffers */
  error = gpgme_data_new_from_mem(&clear_text, SENTENCE, strlen(SENTENCE), 1);
  fail_if_err(error);
  error = gpgme_data_new(&encrypted_text);
  fail_if_err(error); 

  /* Encrypt */
  error = gpgme_op_encrypt(context, recipients, 
			   GPGME_ENCRYPT_ALWAYS_TRUST, clear_text, encrypted_text);
  fail_if_err(error);
  result = gpgme_op_encrypt_result(context);
  if (result->invalid_recipients)
    {
      fprintf (stderr, "Invalid recipient found: %s\n",
	       result->invalid_recipients->fpr);
      exit (1);
    }

  nbytes = gpgme_data_seek (encrypted_text, 0, SEEK_SET);
  if (nbytes == -1) {
    fprintf (stderr, "%s:%d: Error in data seek: ",			
	     __FILE__, __LINE__);
    perror("");
    exit (1);					
    }  
  buffer = malloc(MAXLEN);
  nbytes = gpgme_data_read(encrypted_text, buffer, MAXLEN);
  if (nbytes == -1) {
    fprintf (stderr, "%s:%d: %s\n",			
	     __FILE__, __LINE__, "Error in data read");
    exit (1);					
  }
  buffer[nbytes] = '\0';
  printf("Encrypted text (%i bytes):\n%s\n", (int)nbytes, buffer);
  /* OK */

  exit(0);
}
