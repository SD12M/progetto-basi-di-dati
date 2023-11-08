#include <iostream>
#include <iomanip>
#include <cstdio>
#include <fstream>
#include <stdlib.h>
#include "dependencies/include/libpq-fe.h"

using namespace std;

#define NOMEDB  "Biblioteca"
#define HOST  "127.0.0.1"
#define USER  "postgres"
#define PASS  "postgres"
#define PORT  5432

void printQuery(PGresult* result) {
	cout<< "Risultato: \n" << endl;

	int tuple = PQntuples(result);
	int campi = PQnfields(result);
	
	// Stampa intestazioni
	for (int i = 0; i < campi ; ++ i){
		cout << left << setw(30) << PQfname(result, i);
	}

	cout << "\n\n"; 
	// Stampa valori
	for(int i=0; i < tuple; ++i) {
		for (int n=0; n<campi; ++n) {
			cout << left << setw(30) << PQgetvalue(result, i, n);
		}
		cout << '\n';
	}

	cout<<'\n'<< endl;
	
	PQclear(result);
}

PGresult* eseguiQuery(PGconn* conn, const char* query) {
    PGresult* res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        cout << PQerrorMessage(conn) << endl;
        PQclear(res);
        exit(1);
    }

    return res;
}

int main (int argc, char **argv){
    PGconn* conn;
    char conninfo[250];
    sprintf(conninfo, "user=%s password=%s dbname=%s hostaddr=%s port=%d",
                USER, PASS, NOMEDB, HOST, PORT
    );
    cout<<"Connessione in corso...\n";
    do{
        conn=PQconnectdb(conninfo);
    }while(PQstatus(conn) != CONNECTION_OK);
    cout<<"Connesso\n";

    const char* query[10] = {
        /*1 Ricerca dell’autore che ha avuto più incassi.*/
        "SELECT a.nome, a.cognome, SUM(l.incassi_totali) FROM autore AS a " 
        "INNER JOIN libro as l ON l.autore = a.codice_fiscale "
        "GROUP BY a.nome,a.cognome "
        "ORDER BY SUM(l.incassi_totali) DESC",

        /*2 Visualizzare per ogni noleggio con una sanzione maggiore di 20 euro il motivo.*/
        "SELECT n.id, n.motivazione FROM noleggio AS n "
        "WHERE (n.importo_sanzione > 100)",

        /*3 Per ogni università visualizzare i badge degli studenti che hanno effettuato degli acquisti con un importo maggiore di 100 euro.*/
        "SELECT b.codice_barre, s.nome, s.cognome FROM badge AS b "
        "INNER JOIN studente as s ON b.id=s.badge AND b.universita=s.universita "
        "INNER JOIN acquisto as a ON s.matricola=a.matricola AND s.universita=a.universita "
        "WHERE (a.importo>20) "
        "GROUP BY b.codice_barre, s.nome, s.cognome",

        /*4 Per ogni università visualizzare (per categorie) i libri venduti con un costo maggiore di 30 euro.*/
        "SELECT u.nome, l.genere, a.importo FROM universita AS u "
        "INNER JOIN studente AS s ON u.nome = s.universita "
        "INNER JOIN acquisto AS a ON u.nome = a.universita AND s.matricola=a.matricola "
        "INNER JOIN libro AS l ON a.libro = l.titolo "
        "WHERE a.importo>30 "
        "ORDER BY u.nome, l.genere",

        /*5 Visualizzare i libri in ordine di punteggio medio crescente.*/
        "SELECT l.titolo, l.punteggio_medio FROM libro AS l "
        "ORDER BY l.punteggio_medio ASC",

        /*6 Per ogni università visualizzare i bibliotecari che ci hanno lavorato.*/
        "SELECT u.nome, b.nome, b.cognome FROM universita AS u "
        "INNER JOIN biblioteca AS bib ON u.nome = bib.universita "
        "INNER JOIN lavorato_a AS la ON la.biblioteca = bib.nome "
        "INNER JOIN bibliotecario AS b on b.codice_fiscale = la.bibliotecario "
        "ORDER BY u.nome",

        /*7 Visualizzare la Casa editrice che fornisce più biblioteche.*/
        "SELECT ce.nome, COUNT(DISTINCT bib.nome) FROM casa_editrice AS ce "
        "INNER JOIN fornisce_da AS fd ON ce.partita_iva = fd.casa_editrice "
        "INNER JOIN fornitore AS f ON fd.p_iva_fornitore = f.partita_iva "
        "INNER JOIN fornisce_a AS fa ON f.partita_iva = fa.p_iva_fornitore "
        "INNER JOIN biblioteca AS bib ON fa.biblioteca = bib.nome "
        "GROUP BY ce.nome "
        "ORDER BY COUNT(DISTINCT bib.nome) DESC",

        /*8 Visualizzare le Università in ordine di numero di libri contenuti nelle sue biblioteche.*/
        "SELECT u.nome, b.numero_libri_contenuti FROM universita AS u "
        "INNER JOIN biblioteca AS b ON u.nome=b.universita "
        "GROUP BY u.nome, b.numero_libri_contenuti "
        "ORDER BY b.numero_libri_contenuti DESC",

        /*9 Visualizzare i fondi medi di ogni università.*/
        "SELECT u.nome, AVG(fpub.importo + fpri.importo) FROM universita AS u "
        "INNER JOIN fondo_pubblico AS fpub ON u.nome=fpub.universita "
        "INNER JOIN fondo_privato AS fpri ON u.nome=fpri.universita "
        "GROUP BY u.nome",

        /*10 Per ogni biblioteca visualizzare quelle che contengono più di 20 libri.*/
        "SELECT b.nome, COUNT(c.libro) FROM biblioteca AS b "
        "INNER JOIN scaffale AS s ON b.nome = s.biblioteca "
        "INNER JOIN contiene AS c ON s.biblioteca = c.biblioteca AND s.codice=c.scaffale "
        "GROUP BY b.nome "
        "HAVING (COUNT(c.libro)>20)"
        
    };

    bool terminato=false;
    PGresult* result;
    while(!terminato){
        cout<<"Seleziona la query da eseguire:\n"
        <<"0. Esci\n"
        <<"1. Ricerca dell’autore che ha avuto più incassi.\n"
        <<"2. Visualizzare per ogni noleggio con una sanzione maggiore di 20 euro il motivo.\n"
        <<"3. Per ogni università visualizzare i badge degli studenti che hanno effettuato degli acquisti con un importo maggiore di 100 euro.\n"
        <<"4. Per ogni università visualizzare (per categorie) i libri venduti con un costo maggiore di 30 euro.\n"
        <<"5. Visualizzare i libri in ordine di punteggio medio crescente.\n"
        <<"6. Per ogni università visualizzare i bibliotecari che ci hanno lavorato.\n"
        <<"7. Visualizzare la Casa editrice che fornisce più biblioteche.\n"
        <<"8. Visualizzare le Università in ordine di numero di libri contenuti nelle sue biblioteche.\n"
        <<"9. Visualizzare i fondi medi di ogni università.\n"
        <<"10. Per ogni biblioteca visualizzare quelle che contengono più di 20 libri.\n";
        int scelta=-1;
        cin>>scelta;
        switch (scelta)
        {
        case 0:
            terminato=true;
            exit(1);
            break;
        case 1:
            result=eseguiQuery(conn,query[0]);
            printQuery(result);
            break;
        case 2:
            result=eseguiQuery(conn,query[1]);
            printQuery(result);
            break;
        case 3:
            result=eseguiQuery(conn,query[2]);
            printQuery(result);
            break;
        case 4:
            result=eseguiQuery(conn,query[3]);
            printQuery(result);
            break;
        case 5:
            result=eseguiQuery(conn,query[4]);
            printQuery(result);
            break;
        case 6:
            result=eseguiQuery(conn,query[5]);
            printQuery(result);
            break;
        case 7:
            result=eseguiQuery(conn,query[6]);
            printQuery(result);
            break;
        case 8:
            result=eseguiQuery(conn,query[7]);
            printQuery(result);
            break;
        case 9:
            result=eseguiQuery(conn,query[8]);
            printQuery(result);
            break;
        case 10:
            result=eseguiQuery(conn,query[9]);
            printQuery(result);
            break;
        }
        cout<<"Premi Enter";
        fflush(stdin);
        getchar();
    };
    PQfinish(conn);
}

 
