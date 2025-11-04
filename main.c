#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>


#define MAX_WORD_COUNT 1000
#define MAX_SUCCESSOR_COUNT MAX_WORD_COUNT / 10


// Indlæser hele bogen som en stor tekststreng.
// #embed betyder, at teksten fra "pg84.txt" bliver sat direkte ind her i programmet.
char book[] = {
#embed "pg84.txt"
    , '\0'};

/// Array of tokens registered so far
/// No duplicates are allowed.
char *tokens[MAX_WORD_COUNT];
/// `tokens`'s current size
size_t tokens_size = 0;


//laver array til alle efterfølgere tokensne kan have, og det er 2D array så tokens kan have flere succesors
char *succs[MAX_WORD_COUNT][MAX_SUCCESSOR_COUNT];
// Her gemmes antallet af successors for hvert ord.
size_t succs_sizes[MAX_WORD_COUNT];

// Denne funktion gennemgår bogen og hvis et tegn ikke kan udskrives (f.eks. linjeskift eller specialtegn), bliver det erstattet med et mellemrum.
// Det gør jeg ikke får duplicates
void replace_non_printable_chars_with_space() {
    for(int i = 0; i<strlen(book);i++){
      if(!isprint(book[i])) {
        book[i]=' ';
      }
  }
}

//returnerer tokenid hvis jeg kender den, ellers laver den ny token 
size_t token_id(char *token) {
  size_t id;
  //hvis jeg allerede kender det token jeg inputter returener den bare pladsen den token har
  for (id = 0; id < tokens_size; ++id) {
    if (strcmp(tokens[id], token) == 0) {
      return id;
    }
  }
  //ellers opretter jeg en ny token plads og returnerer den
  tokens[id] = token;
  ++tokens_size;
  return id;
}


/// tilføjer et ord (succ) som en mulig efterfølger til et andet ord (token).
void append_to_succs(char *token, char *succ) {
  auto next_empty_index_ptr = &succs_sizes[token_id(token)];

  //hvis jeg har åpnået max successor sendes fejlkode
  if (*next_empty_index_ptr >= MAX_SUCCESSOR_COUNT) {
    printf("Successor array full.");
    exit(EXIT_FAILURE);
  }

  succs[token_id(token)][(*next_empty_index_ptr)++] = succ;
}

// Opdeler teksten  i tokens. 
// Den bruger de tegn, vi giver som afgrænsere (delimiters), her newline og linjeskift.
// For hvert ord registreres også, hvilket ord der kommer bagefter.

void tokenize_and_fill_succs(char *delimiters, char *str) {
    char *token;
   char *temp;
  token = strtok(str, delimiters); //finder første ord i bogen

  while (token) { //så længe der er flere ord
    token_id(token); //Tilføj ordet til listen, hvis det ikke allerede findes, via funktionen
    temp=token; //gemmer ordet
    token = strtok(NULL, delimiters); //finder næste ord
    if(token!=NULL){
    append_to_succs( temp , token); //gemmer næste ord som succesor af tidligere ord
    }
  }
}

/// returnerer det sidste tegn i en string
char last_char(char *str) {
    size_t len = strlen(str); //finder sætningens længde
    if (len == 0)
        return '\0';  // Returner 0 hvis stringen er tom
    return str[len - 1];  // Returner sidste tegn
}

/// tjekker, om et ord slutter med et  `!`, `?` eller `.`.
bool token_ends_a_sentence(char *token) {
    size_t len = strlen(token); //finder længden af ordet 
    if (len == 0)
        return false;  // tom string kan ikke afslutte en sætning

    char c = token[len - 1]; // finder sidste char
    return (c == '.' || c == '!' || c == '?');//returnerer om det er en af dem
}

// vælger et tilfældigt ord (token), som starter med stort bogstav
size_t random_token_id_that_starts_a_sentence() {

  size_t id;
  do {
    id = rand() % tokens_size; // Vælg et tilfældigt ord
  } while (!isupper((unsigned char)tokens[id][0])); // Gentager indtil den første char i det id(ord) jeg vælger er en uppercase
  return id;
}


// genererer en tilfældig sætning
//Den starter med et ord, der begynder med stort bogstav, og tilføjer derefter tilfældige efterfølgere
// Den stopper, når sætningen bliver for lang eller når et ord slutter med ., ! eller ?.
///  Returnerer fyldt sætning array
char *generate_sentence(char *sentence, size_t sentence_size) {
  //finder startord
  size_t current_token_id = random_token_id_that_starts_a_sentence();
  auto token = tokens[current_token_id];

  strcat(sentence, token); // Tilføj første ord til sætningen
  if (token_ends_a_sentence(token))//hvis ordet afslutter sætning skal den stoppe
    return sentence;
  // Calculated sentence length for the next iteration.
  // Used to stop the loop if the length exceeds sentence size
  size_t sentence_len_next;

  
  do {
    // Find antallet af mulige efterfølgere for det nuværende ord
    size_t succ_count = succs_sizes[current_token_id];
    if (succ_count == 0)
        break; // ingen efterfølgere

    // vælg en tilfældig efterfølger blandt de mulige og opdater 
    size_t next_id = rand() % succ_count;
    char *next_token = succs[current_token_id][next_id];

    // beregn længden, hvis vi tilføjer næste ord
    sentence_len_next = strlen(sentence) + strlen(next_token) + 2; // +2 for mellemrum + '\0'
    
    if (sentence_len_next >= sentence_size - 1)
        break; // ikke plads til næste ordså stopper den

    // tilføj mellemrum og næste token
    strcat(sentence, " ");
    strcat(sentence, next_token);
    
    // stop hvis ordet afslutter en sætning
    if (token_ends_a_sentence(next_token))
        break;

    // opdater current_token_id til id af det ord jeg har valgt som næste
    current_token_id = token_id(next_token);

  } while (sentence_len_next < sentence_size - 1);

  return sentence;
}



int main() {
  replace_non_printable_chars_with_space();

  char *delimiters = " \n\r";// Afgrænsere: mellemrum og linjeskift
  tokenize_and_fill_succs(delimiters, book);// Del bogen op i ord 

  char sentence[1000];// Buffer til at gemme sætninger i
  srand(time(NULL));// Sørger for, at programmet laver forskellige resultater hver gang


  // Generer et spørgsmål og printer det
  do {
    memset(sentence, '\0', sizeof(sentence)); // Tøm sætningen
    generate_sentence(sentence, sizeof sentence);//lav ny sætning
  } while (last_char(sentence) != '?');//gentag indtil sætning stuller med ?
  puts(sentence);
  puts("");

  // Generer en sætning, der ender med udråbstegn og printer
  do {
    memset(sentence, '\0', sizeof(sentence));
    generate_sentence(sentence, sizeof sentence);
  } while (last_char(sentence) != '!');
  puts(sentence);
}

