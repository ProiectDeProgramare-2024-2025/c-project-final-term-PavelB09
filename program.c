#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLUE "\033[1;34m"
#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m"
#define MAGENTA "\033[1;35m"
#define YELLOW "\033[1;33m"
#define RED "\033[1;31m"
#define RESET "\033[0m"

typedef struct
{
    char nume[30];
    char prenume[30];
    char specialitate[30];
    char program[20]; // 10:00-12:00
} Medic;

typedef struct
{
    char numePacient[30];
    char prenumePacient[30];
    char telefon[15];
    Medic medic;
    char data[15];
    char ora[10];
} Programare;

// Functie ce crtata ecranul
void clearScreen()
{
#ifdef _WIN32
    system("cls"); // Windows
#else
    system("clear"); // macos/Linux
#endif
}

// Goleste bufferul de intrare
void golesteBuf()
{
    while (getchar() != '\n')
        ;
}

// Validare data (DD/MM/YYYY)
int esteDataValida(const char *data)
{
    int zi, luna, an;
    if (sscanf(data, "%2d/%2d/%4d", &zi, &luna, &an) != 3)
        return 0;
    if (zi < 1 || zi > 31 || luna < 1 || luna > 12 || an < 2025)
        return 0;

    time_t now = time(NULL);                         // Obtine timpul curent
    struct tm today = *localtime(&now);              // Obtine structura tm pentru timpul curent
    today.tm_hour = today.tm_min = today.tm_sec = 0; // Resetam ora pentru comparatie (punem la 00:00:00)

    struct tm input = {0}; // Initializam structura tm pentru data de intrare
    input.tm_mday = zi;
    input.tm_mon = luna - 1;
    input.tm_year = an - 1900;

    time_t t_today = mktime(&today); // Obtine timpul curent in secunde
    time_t t_input = mktime(&input); // Obtine timpul de intrare in secunde
    return (t_input >= t_today);
}

// Validare ora (HH:MM)
int esteOraValida(const char *ora)
{
    int h, m;
    if (sscanf(ora, "%2d:%2d", &h, &m) != 2)
        return 0;
    if (h < 0 || h > 23 || m < 0 || m > 59)
        return 0;
    return 1;
}

// Citeste input validat cu functie de validare
void citesteInputValidat(const char *text, char *buffer, int size, int (*valid)(const char *))
{
    do
    {
        printf("%s", text);
        if (!fgets(buffer, size, stdin))
            return;
        buffer[strcspn(buffer, "\n")] = '\0';
        if (valid(buffer))
            break;
        printf("Format invalid / data din trecut, astept: %s\n", text);
    } while (1);
}

// Citeste input
void citesteInput(const char *text, char *buffer, int size)
{
    printf("%s", text);
    fgets(buffer, size, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
}

// Afiseaza lista de specialitati disponibile
void listareSpecialitati()
{
    FILE *fp = fopen("medici.txt", "r");
    if (!fp)
    {
        printf("Nu exista lista de medici.\n");
        return;
    }
    char linie[256], nm[30], pr[30], sp[30], prg[20];
    char speci[100][30];
    int count = 0;

    fgets(linie, sizeof(linie), fp);

    while (fgets(linie, sizeof(linie), fp))
    {
        if (sscanf(linie, "%[^,],%[^,],%[^,],%[^\n]", nm, pr, sp, prg) == 4)
        {
            int exista = 0;
            for (int i = 0; i < count; i++)
            {
                if (strcmp(speci[i], sp) == 0)
                {
                    exista = 1;
                    break;
                }
            }
            if (!exista && count < 100)
            {
                strcpy(speci[count++], sp);
            }
        }
    }
    fclose(fp);

    printf("Specialitati existente:\n");
    for (int i = 0; i < count; i++)
    {
        printf(" %2d. %s\n", i + 1, speci[i]);
    }
    printf("\n");
    printf("Apasati Enter pentru a continua...\n");
    golesteBuf();

}

// Converteste timpul din format "HH:MM" in minute
int convertesteTimpInMinute(const char *timpStr)
{
    int ore, minute;
    if (sscanf(timpStr, "%d:%d", &ore, &minute) == 2)
    {
        return ore * 60 + minute;
    }
    return -1;
}

// Verifica daca ora data se incadreaza in programul medicului
int esteInProgram(const char *programMedic, const char *timp)
{
    char oraInceput[6], oraFinal[6]; // Ora de inceput si ora de final a unui program
    if (sscanf(programMedic, "%5[^-]-%5s", oraInceput, oraFinal) != 2)
    {
        return 0;
    }
    int inceputMinute = convertesteTimpInMinute(oraInceput);
    int finalMinute = convertesteTimpInMinute(oraFinal);
    int timpMinute = convertesteTimpInMinute(timp);
    if (timpMinute == -1 || inceputMinute == -1 || finalMinute == -1)
    {
        return 0;
    }
    return (timpMinute >= inceputMinute && timpMinute < finalMinute);
}

// Verifica daca exista conflict intre programari (daca deja exista un pacient care are programare la acel medic in acel interval orar)
int conflictProgramare(Medic doctor, const char *data, const char *timp)
{
    FILE *fp = fopen("programari.txt", "r");
    if (fp == NULL)
        return 0;

    char linie[256];
    fgets(linie, sizeof(linie), fp); // dau skip prima linie
    Programare p;
    int conflict = 0;

    while (fgets(linie, sizeof(linie), fp) != NULL)
    {
        if (sscanf(linie, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^\n]",
                   p.numePacient, p.prenumePacient, p.telefon,
                   p.medic.nume, p.medic.prenume,
                   p.medic.specialitate, p.medic.program, p.data, p.ora) == 9)
        {
            if (strcmp(p.medic.nume, doctor.nume) == 0 &&
                strcmp(p.medic.prenume, doctor.prenume) == 0 &&
                strcmp(p.data, data) == 0 &&
                strcmp(p.ora, timp) == 0)
            {
                conflict = 1;
                break;
            }
        }
    }
    fclose(fp);
    return conflict;
}

// Citeste optiunea de la tastatura
int readOption()
{
    int opt;
    scanf("%d", &opt);
    golesteBuf();
    return opt;
}

// crtata ecranul si afiseaza header-ul
void printHeader(const char *header)
{
    clearScreen();
    printf("%s<---%s %s %s--->%s\n\n", CYAN, RESET, header, CYAN, RESET);
}

// Asteapta Enter pentru a reveni la meniu
void revenireLaMeniu()
{
    printf("Apasati Enter pentru a reveni...");
    getchar();
}

// Afiseaza un meniu simplu si returneaza optiunea (0 pentru revenire, 1 pentru actiune)
int fOptiune(const char *actiune)
{
    int opt;
    do
    {
        printHeader(actiune);
        /*
          printf("Optiuni:\n");
          printf("1. %s\n", actiune);
          printf("0. Revenire\n");
          printf("Introduceti optiunea: ");
        */
        printf("Optiuni:\n");
        printf("%s1.%s %s \n", GREEN, RESET, actiune);
        printf("%s0.%s Revenire\n", RED, RESET);
        printf("Introduceti optiunea: ");
        opt = readOption();
        if (opt == 0 || opt == 1)
        {
            clearScreen();
            return opt;
        }
        printf("%s Optiune invalida! %s\n", RED, RESET);
        revenireLaMeniu();
    } while (1);
}

// Afiseaza lista de programari din fisierul programari.txt
void afiseazaProgramari()
{
    int optiune;
    char linie[256];

    do
    {
        if ((optiune = fOptiune("Afiseaza programari")) == 0)
        {
            break;
        }
        FILE *fp = fopen("programari.txt", "r");
        if (fp == NULL)
        {
            printf("Nu exista programari salvate.\n");
        }
        else
        {
            fgets(linie, sizeof(linie), fp);
            Programare p;
            int k = 1;
            int gasit = 0;
            while (fgets(linie, sizeof(linie), fp) != NULL)
            {
                if (sscanf(linie, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^\n]",
                           p.numePacient, p.prenumePacient, p.telefon,
                           p.medic.nume, p.medic.prenume,
                           p.medic.specialitate, p.medic.program, p.data, p.ora) == 9)
                {
                    gasit = 1;
                    printf("%sProgramare %d:%s\n", CYAN, k, RESET);
                    printf(" %sPacient:%s %s %s\n", BLUE, RESET, p.numePacient, p.prenumePacient);
                    printf(" %sTelefon:%s %s\n", GREEN, RESET, p.telefon);
                    printf(" %sMedic:%s %s %s\n", MAGENTA, RESET, p.medic.nume, p.medic.prenume);
                    printf(" %sSpecialitate:%s %s\n", YELLOW, RESET, p.medic.specialitate);
                    printf(" %sProgram medic:%s %s\n", YELLOW, RESET, p.medic.program);
                    printf(" %sData:%s %s\n", BLUE, RESET, p.data);
                    printf(" %sOra:%s %s\n", GREEN, RESET, p.ora);
                    printf("-----------------------------\n");
                    k++;
                }
                else
                {
                    printf("Linie invalida: %s\n", linie);
                }
            }
            if (!gasit)
            {
                printf("Nu exista programari salvate.\n");
            }
            fclose(fp);
        }
        revenireLaMeniu();
    } while (1);
}

// Cauta un medic dupa criterii: nume, specialitate si program
void cautaMedic()
{
    int opt;
    do
    {
        if ((opt = fOptiune("Cauta medic")) == 0)
            break;

        printf("Introduceti criterii de cautare (lasati necompletat pentru a ignora criteriul):\n");
        char nume[30], specialitate[30], program[20];
        char linie[256];
        Medic medic;

        citesteInput("Nume: ", nume, sizeof(nume));
        citesteInput("Specialitate: ", specialitate, sizeof(specialitate));
        citesteInput("Program (ex: 10:00-12:00): ", program, sizeof(program));
        printf("\n");

        if (nume[0] == '\0' && specialitate[0] == '\0' && program[0] == '\0')
        {
            printf("Nu ati specificat niciun criteriu de cautare. Revenire la meniul principal.\n");
            revenireLaMeniu();
            continue;
        }

        FILE *fp = fopen("medici.txt", "r");
        if (fp == NULL)
        {
            printf("Nu exista lista de medici salvata.\n");
            revenireLaMeniu();
            break;
        }
        fgets(linie, sizeof(linie), fp);
        int k = 0;
        while (fgets(linie, sizeof(linie), fp) != NULL)
        {
            int gasit = 1;
            if (sscanf(linie, "%[^,],%[^,],%[^,],%[^\n]",
                       medic.nume, medic.prenume, medic.specialitate, medic.program) == 4)
            {
                if (strlen(nume) > 0 && strcmp(nume, medic.nume) != 0)
                {
                    gasit = 0;
                }
                if (strlen(specialitate) > 0 && strcmp(specialitate, medic.specialitate) != 0)
                {
                    gasit = 0;
                }
                if (strlen(program) > 0 && strcmp(program, medic.program) != 0)
                {
                    gasit = 0;
                }
                if (gasit == 1)
                {
                    k++;
                    printf("%sMedic %d:%s\n", MAGENTA, k, RESET);
                    printf(" %sNume:%s %s %s\n", BLUE, RESET, medic.nume, medic.prenume);
                    printf(" %sSpecialitate:%s %s\n", YELLOW, RESET, medic.specialitate);
                    printf(" %sProgram:%s %s\n", CYAN, RESET, medic.program);
                    printf("---------------------------\n");
                }
            }
            else
            {
                printf("Linie invalida: %s\n", linie);
            }
        }
        fclose(fp);

        if (k == 0)
        {
            printf("Nu s-a gasit niciun medic care sa corespunda criteriilor.\n");
        }

        revenireLaMeniu();
    } while (1);
}

// Adauga o programare noua, verificand disponibilitatea medicului
void adaugaProgramare()
{
    int opt;
    do
    {
        if ((opt = fOptiune("Adauga programare")) == 0)
        {
            break;
        }
        char numePacient[30], prenumePacient[30], telefon[15], specialitate[30], data[15], timp[10];

        listareSpecialitati();
        clearScreen();

        citesteInput("Nume pacient: ", numePacient, sizeof(numePacient));
        citesteInput("Prenume pacient: ", prenumePacient, sizeof(prenumePacient));
        citesteInput("Telefon pacient: ", telefon, sizeof(telefon));
        citesteInput("Specialitate dorita: ", specialitate, sizeof(specialitate));
        citesteInputValidat("Data programarii (DD/MM/YYYY): ", data, sizeof(data), esteDataValida);
        citesteInputValidat("Ora programarii (HH:MM): ", timp, sizeof(timp), esteOraValida);

        FILE *fp = fopen("medici.txt", "r");
        if (fp == NULL)
        {
            printf("Nu exista lista de medici salvata.\n");
            revenireLaMeniu();
            break;
        }

        // Verific daca medicul este disponibil
        Medic medicSelectat;
        char linie[256];
        int medicGasit = 0;
        fgets(linie, sizeof(linie), fp);
        while (fgets(linie, sizeof(linie), fp) != NULL && !medicGasit)
        {
            Medic medic;
            if (sscanf(linie, "%[^,],%[^,],%[^,],%[^\n]",
                       medic.nume, medic.prenume, medic.specialitate, medic.program) == 4)
            {
                if (strcmp(specialitate, medic.specialitate) == 0 &&
                    esteInProgram(medic.program, timp) &&
                    !conflictProgramare(medic, data, timp))
                {
                    medicSelectat = medic;
                    medicGasit = 1;
                }
            }
        }
        fclose(fp);
        if (!medicGasit)
        {
            printf("Nu s-a gasit niciun medic disponibil cu specialitatea %s la ora %s sau este deja ocupata.\n", specialitate, timp);
            revenireLaMeniu();
            break;
        }

        FILE *fprog = fopen("programari.txt", "a");
        if (fprog == NULL)
        {
            printf("Eroare la deschiderea fisierului programari.txt.\n");
            revenireLaMeniu();
            break;
        }
        fprintf(fprog, "\n%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                numePacient, prenumePacient, telefon,
                medicSelectat.nume, medicSelectat.prenume,
                medicSelectat.specialitate, medicSelectat.program, data, timp);
        fclose(fprog);
        printf("Programare adaugata cu succes!\n");
        revenireLaMeniu();
        break;
    } while (1);
}

// Sterge o programare selectata de utilizator
void stergeProgramare()
{
    int opt;
    do
    {
        if ((opt = fOptiune("Sterge programare")) == 0)
            break;
        FILE *fp = fopen("programari.txt", "r");
        if (!fp)
        {
            printf("Nu exista programari.\n");
            revenireLaMeniu();
            break;
        }

        char buffer[256];
        fgets(buffer, sizeof(buffer), fp);
        int cnt = 0;
        // afisare programari
        while (fgets(buffer, sizeof(buffer), fp))
            printf("%d. %s", ++cnt, buffer);
        fclose(fp);
        if (!cnt)
        {
            printf("Nu exista programari.\n");
            revenireLaMeniu();
            break;
        }
        printf("\n Numar programare de sters (0-revenire): ");
        int nr = readOption();
        if (nr == 0)
            break;
        if (nr < 1 || nr > cnt)
        {
            printf("Numar invalid.\n");
            revenireLaMeniu();
            break;
        }
        FILE *ft = fopen("temp.txt", "w");
        if (!ft)
        {
            printf("Eroare.\n");
            revenireLaMeniu();
            break;
        }
        fp = fopen("programari.txt", "r");
        fgets(buffer, sizeof(buffer), fp); 
        fputs(buffer, ft);                 
        int crt = 0;
        while (fgets(buffer, sizeof(buffer), fp))
        {
            if (++crt != nr)
                fputs(buffer, ft);
        }
        fclose(fp);
        fclose(ft);
        remove("programari.txt");
        rename("temp.txt", "programari.txt");
        printf("%sProgramare stearsa cu succes!%s\n", MAGENTA, RESET);
        revenireLaMeniu();
        break;
    } while (1);
}

// Afiseaza meniul principal al aplicatiei
void meniuPrincipal()
{
    printf("%s<---%s Programare consultatie %s--->%s \n", CYAN, RESET, CYAN, RESET);
    printf("%s1.%s Afiseaza programari\n", GREEN, RESET);
    printf("%s2.%s Cauta medic\n", GREEN, RESET);
    printf("%s3.%s Adauga programare\n", GREEN, RESET);
    printf("%s4.%s Sterge programare\n", GREEN, RESET);
    printf("%s0.%s Iesire\n", RED, RESET);
    printf("Introduceti optiunea: ");
}

int main()
{
    int optiune;
    do
    {
        clearScreen();
        meniuPrincipal();
        optiune = readOption();

        switch (optiune)
        {
        case 1:
            afiseazaProgramari();
            break;
        case 2:
            cautaMedic();
            break;
        case 3:
            adaugaProgramare();
            break;
        case 4:
            stergeProgramare();
            break;
        case 0:
            clearScreen();
            printf("Ati inchis aplicatia. La revedere!\n");
            break;
        default:
            printf("Optiune invalida! Apasati Enter pentru a reveni.\n");
            getchar();
            break;
        }
    } while (optiune != 0);

    return 0;
}
