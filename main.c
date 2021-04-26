/*
 ----- Structuri de date si algoritmi. Laborator -----
 -----      Miclos Eduard-Pavel, proiect #6.     -----
 -----    Universitatea Politehnica Timisoara    -----
 ----- Facultatea de Automatica si Calculatoare  -----
 -----            Informatica, an 1              -----
*/

/* ---- Librarii incluse ---- */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* -------------------------- */

/* ---- Macrouri ----*/
#define ALOCA(TIP, N) (TIP*)malloc(sizeof(TIP) * (N))
#define REALOCA(X, TIP, N) (TIP*)realloc(X, sizeof(TIP)*(N))
/* -----------------*/

/* ---- Enum si structuri ---- */
typedef enum { alfabetic, de_la_producator, categorie } conditie_afisare;

typedef struct joc {
	char* nume, *categorie;
	float pret;
	unsigned stoc;
	struct joc* urm, * ant;
} joc;

typedef struct producator {
	char* nume;
	struct producator* urm, *ant;
	int nr_jocuri;
	joc* prim;
} prod;
/* ------------------------ */

/* ---- Variabile globale ---- */
prod* rad = NULL;
int nr_producatori = 0;
/* --------------------------- */

/* ---- Prototipuri functii ---- */
char* citeste_cuvant(FILE*);
prod* citeste_prod(FILE*);
joc* citeste_joc(FILE*);
void modifica_date_joc(joc*, joc*);
joc* adauga_joc(joc*, joc*);
prod* adauga(prod*, prod*, joc*);
void citire(void);

void elibereaza(char***, int);
void meniu(void);
void afisare_coloane(FILE*);
void afisare_linie(FILE*, prod*, joc*);
void afisare();
void quick_sort(char**, int, int);
prod* exista_producator(char*);
void afisare_conditionata(conditie_afisare);
void modifica_lista();
void calculare_valoare_stoc();
void cautare_joc();
void salvare_in_fisier();
/* --------------------------- */

int main() {
	int optiune;

	citire();
	meniu();
	afisare();

	do {
		printf("Optiune: ");
		scanf("%d", &optiune);

		switch (optiune) {
		case 1:
			afisare_conditionata(alfabetic);
			break;
		case 2:
			afisare_conditionata(de_la_producator);
			break;
		case 3:
			afisare_conditionata(categorie);
			break;
		case 4:
			modifica_lista();
			break;
		case 5:
			calculare_valoare_stoc();
			break;
		case 6:
			cautare_joc();
			break;
		case 7:
			salvare_in_fisier();
			break;
		default:
			break;
		}

	} while (optiune);


	printf("\nAti parasit programul.\n");

	return 0;
}

/* ---- Functii ---- */
char* citeste_cuvant(FILE* f) {
	char* cuv = NULL, * aux = NULL;
	int n = 0;
	char c;

	while (1) {
		n++;
		if (!(aux = REALOCA(aux, char, n))) {
			printf("Eroare la alocarea in memorie.");
			exit(EXIT_FAILURE);
		}
		cuv = aux;
		/* Cuvantul e citit din fisierul f (stdin sau jocuri.in, dupa caz) */
		c = fgetc(f);
		/* Citim pana la intalnirea spatiului sau pana la newline. */
		if (c == ' ' || c == '\n') break;
		cuv[n - 1] = c;
	}
	cuv[n - 1] = '\0';
	return cuv;
}

prod* citeste_prod(FILE* f) {
	prod* prd;
	if (!(prd = ALOCA(prod, 1))) {
		printf("Eroare la alocarea in memorie.");
		exit(EXIT_FAILURE);
	}
	prd->nume = citeste_cuvant(f);
	return prd;
}

joc* citeste_joc(FILE* f) {
	joc* jc;

	if (!(jc = ALOCA(joc, 1))) {
		fprintf(stderr, "Eroare la deschiderea fisierului.");
		exit(EXIT_FAILURE);
	}

	jc->nume = citeste_cuvant(f);
	jc->categorie = citeste_cuvant(f);
	fscanf(f, "%f", &jc->pret);
	fscanf(f, "%u", &jc->stoc);
	return jc;
}

void modifica_date_joc(joc *inainte, joc *dupa) {
	char* aux = inainte->categorie;
	
	if (!(aux = REALOCA(aux, char, strlen(dupa->categorie) + 1))) {
		fprintf(stderr, "Erorare la alocarea in memorie.");
		exit(EXIT_FAILURE);
	}
	inainte->categorie = aux;
	strcpy(inainte->categorie, dupa->categorie);
	inainte->pret = dupa->pret;
	inainte->stoc = dupa->stoc;
}

joc* adauga_joc(joc* rad, joc* jc) {
	joc* q = rad;
	jc->urm = NULL;
	if (q) {
		while (q->urm && strcmp(q->nume, jc->nume)) q = q->urm;
		/*In cazul in care jocul exista deja, modificam datele si eliberam jc. */
		if (!strcmp(q->nume, jc->nume)) {
			modifica_date_joc(q, jc);
			free(jc);
		}
		else {
		/* Altfel, adaugam jc la sfarsitul listei. */
			q->urm = jc;
			jc->ant = q;
		}
		return rad;
	}

	/* Daca q este NULL, inseamna ca nu exista jocuri => introducem radacina => nodul anterior e NULL. */
	jc->ant = NULL;
	return jc;
}

prod* adauga(prod* rad, prod* prd, joc* jc) {
	prod* q1, * q2;
	for (q1 = q2 = rad; q1 != NULL && strcmp(q1->nume, prd->nume); q2 = q1, q1 = q1->urm);
	if (q1 != NULL && !strcmp(q1->nume, prd->nume)) {
		/* Cazul in care producatorul exista deja. Adaugam jocul si eliberam prd.*/
		q1->prim = adauga_joc(q1->prim, jc);
		free(prd);
		q1->nr_jocuri++;
		return rad;
	}
	if (q1 == q2) rad = prd;
	else
		q2->urm = prd;

	/* Altfel, introducem un producator nou. */
	prd->ant = q2;
	prd->urm = NULL;
	prd->prim = adauga_joc(NULL, jc);

	/* Orice producator, odata introdus, va avea un joc in sublista. Restul, daca e cazul, sunt introduse ulterior. */
	nr_producatori++;
	prd->nr_jocuri = 1;

	return rad;
}

void citire(void) {
	FILE* f;
	if (!(f = fopen("jocuri.in", "rt"))) {
		fprintf(stderr, "Eroare la deschiderea fisierului.");
		exit(EXIT_FAILURE);
	}

	prod* prd;
	joc* jc;

	while (!feof(f)) {
		prd = citeste_prod(f);
		jc = citeste_joc(f); fgetc(f);
		rad = adauga(rad, prd, jc);
	}

	fclose(f);
}

void elibereaza(char*** txt, int N) {
	int i;
	for (i = 0; i < N; i++)
		free((*txt)[i]);
	free(*txt);
}

void meniu(void) {
	printf("--- BUN VENIT: GAMING SHOP, BAZA DE DATE ---\n");
	printf("1. Afiseaza lista de producatori in ordine alfabetica.\n");
	printf("2. Afiseaza lista de jocuri de la un producator in ordine alfabetica.\n");
	printf("3. Afiseaza toate jocurile apartinand unei categorii citite de la tastatura.\n");
	printf("4. Modifica / adauga un joc in lista.\n");
	printf("5. Calculare valoare stoc.\n");
	printf("6. Cautare joc dupa titlu si afisare informatii.\n");
	printf("7. Salvarea in fisier a informatiilor din lista cu subliste.\n");
	printf("0. Parasirea programului.\n");
}

void afisare_coloane(FILE *f) {
	/* Afisam in fisierul f (stdout sau jocuri.out, dupa caz). */
	int i;
	fprintf(f, "\n%30s%30s%30s%30s%30s\n%20s", "Producator", "Nume joc", "Categorie", "Pret", "Stoc", "");
	for (i = 0; i < 130; i++) fprintf(f, "-");
	fprintf(f, "\n");
}

void afisare_linie(FILE *f, prod* prd, joc* jc) {
	/* Afisam in fisierul f (stdout sau jocuri.out, dupa caz). */
	fprintf(f, "%30s%30s%30s%30.2f%30u\n", prd->nume, jc->nume, jc->categorie, jc->pret, jc->stoc);
}

void afisare() {
	prod* prd = rad;
	joc* jc;

	afisare_coloane(stdout);
	while (prd) {
		jc = prd->prim;
		while (jc) {
			afisare_linie(stdout, prd, jc);
			jc = jc->urm;
		}
		prd = prd->urm;
	}
}

void quick_sort(char** txt, int s, int d) {
	int i = s, j = d;
	char* mid = txt[(s + d) / 2], * aux = NULL;

	do {
		while (i < d && strcmp(txt[i], mid) < 0) i++;
		while (j > s && strcmp(txt[j], mid) > 0) j--;
		if (i <= j) {
			/* Pt. a efectua interschimbari cat mai econome, realocam in memorie exact numarul necesar de octeti. */
			if (!(aux = REALOCA(aux, char, strlen(txt[i]) + 1))) {
				fprintf(stderr, "Eroare la alocare in memorie.");
				exit(EXIT_FAILURE);
			}

			strcpy(aux, txt[i]);

			if (!(txt[i] = REALOCA(txt[i], char, strlen(txt[j]) + 1))) {
				fprintf(stderr, "Eroare la alocare in memorie.");
				exit(EXIT_FAILURE);
			}

			strcpy(txt[i], txt[j]);

			if (!(txt[j] = REALOCA(txt[j], char, strlen(aux) + 1))) {
				fprintf(stderr, "Eroare la alocare in memorie.");
				exit(EXIT_FAILURE);
			}

			strcpy(txt[j], aux);
			i++; j--;
		}
	} while (i <= j);
	if (j > s)
		quick_sort(txt, s, j);
	if (i < d)
		quick_sort(txt, i, d);
	free(aux);
}

prod* exista_producator(char* producator) {
	prod* prd = rad;
	while (prd) {
		if (!strcmp(prd->nume, producator)) break;
		prd = prd->urm;
	}
	return prd;
}

void afisare_conditionata(conditie_afisare ca) {
	switch (ca) {
	case alfabetic: {
		char** producatori;

		if (!(producatori = ALOCA(char*, nr_producatori))) {
			fprintf(stderr, "Eroare la alocare in memorie.");
			exit(EXIT_FAILURE);
		}
		int i;
		prod* prd = rad;

		for (i = 0; i < nr_producatori; i++) {
			if (!(producatori[i] = ALOCA(char, strlen(prd->nume) + 1))) {
				printf("Eroare la alocare in memorie.");
				exit(EXIT_FAILURE);
			}
			strcpy(producatori[i], prd->nume);
			prd = prd->urm;
		}

		quick_sort(producatori, 0, nr_producatori - 1);
		printf("Lista producatorilor in ordine alfabetica este: ");
		for (i = 0; i < nr_producatori; i++) {
			if (i < nr_producatori - 1)
				printf("%s, ", producatori[i]);
			else
				printf("%s.", producatori[i]);
		}
		printf("\n");

		elibereaza(&producatori, nr_producatori);

		break;
	}
	case de_la_producator: {
		char* nume_prod;
		printf("Introduceti producatorul: ");
		getchar();
		nume_prod = citeste_cuvant(stdin);

		prod* prd;
		if (prd = exista_producator(nume_prod)) {
			char** jocuri;
			int i;
			if (!(jocuri = ALOCA(char*, prd->nr_jocuri))) {
				printf("Eroare la alocarea in memorie.");
				exit(EXIT_FAILURE);
			}

			joc* jc = prd->prim;
			for (i = 0; i < prd->nr_jocuri; i++) {
				if (!(jocuri[i] = ALOCA(char, strlen(jc->nume) + 1))) {
					printf("Eroare la alocarea in memorie.");
					exit(EXIT_FAILURE);
				}
				strcpy(jocuri[i], jc->nume);
				jc = jc->urm;
			}

			quick_sort(jocuri, 0, prd->nr_jocuri - 1);
			printf("Lista jocurilor de la producatorul %s, in ordine alfabetica: ", prd->nume);
			for (i = 0; i < prd->nr_jocuri; i++) {
				if (i < prd->nr_jocuri - 1)
					printf("%s, ", jocuri[i]);
				else
					printf("%s.", jocuri[i]);
			}
			printf("\n");
			elibereaza(&jocuri, prd->nr_jocuri);
			free(nume_prod);
		}
		else
			printf("Nu exista producatorul %s in baza de date.\n", nume_prod);
		break;
	}
	case categorie: {
		char* cat;
		printf("Introduceti categoria: ");
		getchar();
		cat = citeste_cuvant(stdin);

		prod* prd = rad;
		joc* jc;
		int exista_cat = 0;

		while (prd) {
			jc = prd->prim;
			while (jc) {
				if (!strcmp(jc->categorie, cat)) {
					if (!exista_cat) {
						printf("Din categoria '%s' fac parte urmatoarele jocuri: ", cat);
						exista_cat = 1;
						printf("%s", jc->nume);
					}
					else
						printf(", %s", jc->nume);
				}
				jc = jc->urm;
			}
			prd = prd->urm;
		}
		
		if (!exista_cat)
			printf("Nu exista categoria '%s'.", cat);
		printf("\n");

		free(cat);
		break;
	}
	default: break;
	}
}

void modifica_lista() {
	prod* prd;
	joc* jc;

	printf("Introduceti datele in ordinea: nume_producator nume_joc categorie pret stoc.\n");
	getchar();

	prd = citeste_prod(stdin);
	jc = citeste_joc(stdin); getchar();

	rad = adauga(rad, prd, jc);
	afisare();
}

void calculare_valoare_stoc() {
	prod* prd = rad;
	joc* jc;

	/* Formula: suma produselor de forma: pret_joc * stoc */
	float val = 0;
	while (prd) {
		jc = prd->prim;
		while (jc) {
			val = val + jc->pret*jc->stoc;
			jc = jc->urm;
		}
		prd = prd->urm;
	}
	
	printf("Valorea totala a stocului este de: %.2f lei.\n", val);
}

void cautare_joc() {
	char* titlu_joc;
	printf("Introduceti titlul jocului: ");
	getchar();
	titlu_joc = citeste_cuvant(stdin);

	prod* prd = rad;
	joc* jc;
	int gasit = 0;

	while (prd && !gasit) {
		jc = prd->prim;
		while (jc) {
			if (!strcmp(jc->nume, titlu_joc)) {
				afisare_coloane(stdout);
				afisare_linie(stdout, prd, jc);
				gasit = 1;
				break;
			}
			jc = jc->urm;
		}
		prd = prd->urm;
	}

	if (!gasit)
		printf("Jocul cu numele '%s' nu exista in baza de date.\n", titlu_joc);
}

void salvare_in_fisier() {
	FILE* f;

	if (!(f = fopen("jocuri.out", "wt"))) {
		printf("Eroare la deschiderea fisierului.");
		exit(EXIT_FAILURE);
	}

	prod* prd = rad;
	joc* jc;

	afisare_coloane(f);

	while (prd) {
		jc = prd->prim;
		while (jc) {
			afisare_linie(f, prd, jc);
			jc = jc->urm;
		}
		prd = prd->urm;
	}
	
	fclose(f);
}
/* ------------------ */
