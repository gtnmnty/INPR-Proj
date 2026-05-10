#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_ROOMS     100
#define MAX_AMENITIES 20

// ─── Structs ───────────────────────────────────────────────────────────────────

typedef struct {
    int   roomNumber;
    char  category[30];
    int   bedrooms;
    float pricePerNight;
    int   isAvailable;
} InqRoom;

typedef struct {
    char  code[5];
    char  name[60];
    float price;
    char  type[20];
} InqAmenity;

// ─── Forward Declarations ─────────────────────────────────────────────────────

void inquiryMenu();
void inquiryRates();
void inquiryAmenities();
void inquiryRoomAvailability();
void inquiryTryAgain(void (*menuFunc)());

int  inqReadRooms(InqRoom *rooms);
int  inqReadAmenities(const char *filename, InqAmenity *list, int maxCount);
void inqPrintWithCommas(float amount);
char inqPickCategory(const char *prompt);
void inqFormatPrice(float amount, char *buffer);

// ─── Entry Point ──────────────────────────────────────────────────────────────

int main() {
    inquiryMenu();
    return 0;
}

// ─── Inquiry Menu ─────────────────────────────────────────────────────────────

void inquiryMenu() {
    int pick = 0;

    do {
        printf("\n========================================\n");
        printf("         ESPLENIN HOTEL - INQUIRY\n");
        printf("========================================\n");
        printf("  [1] - Room Rates\n");
        printf("  [2] - Amenities\n");
        printf("  [3] - Room Availability\n");
        printf("  [0] - Exit\n");
        printf("========================================\n");
        printf("Choice: ");

        if (scanf("%d", &pick) == 0) {
            while (getchar() != '\n');
            printf("Invalid input.\n");
            pick = -1;
            continue;
        }
        while (getchar() != '\n');

        switch (pick) {
            case 1: inquiryRates();            break;
            case 2: inquiryAmenities();        break;
            case 3: inquiryRoomAvailability(); break;
            case 0: printf("Thank you! Have a nice day.\n"); break;
            default: printf("Invalid choice.\n");
        }

    } while (pick != 0);
}

// ─── Level 1: Rates ───────────────────────────────────────────────────────────

void inquiryRates() {
    InqRoom rooms[MAX_ROOMS];
    int     roomCount = inqReadRooms(rooms);

    if (roomCount == 0) { printf("No room data found.\n"); return; }

    char again = 'y';
    while (tolower(again) == 'y') {

        // Level 2: Pick category
        char categoryChoice = inqPickCategory("\nWhich room category are you interested in?");
        if (categoryChoice == '0') return;

        const char *chosenCategory;
        switch (categoryChoice) {
            case 'A': chosenCategory = "De Luxe";      break;
            case 'B': chosenCategory = "Suite";         break;
            case 'C': chosenCategory = "Luxury Suite";  break;
            default:  printf("Invalid choice.\n"); continue;
        }

        // Find max bedrooms available in this category
        int maxBedrooms = 0;
        for (int i = 0; i < roomCount; i++) {
            if (strcmp(rooms[i].category, chosenCategory) == 0 &&
                rooms[i].bedrooms > maxBedrooms)
                maxBedrooms = rooms[i].bedrooms;
        }

        if (maxBedrooms == 0) {
            printf("No rooms found under %s.\n", chosenCategory);
            goto ratesAskAgain;
        }

        // Level 3: Pick bedroom count
        int bedroomPick = 0;
        do {
            printf("How many bedrooms? (1 - %d, max for %s): ", maxBedrooms, chosenCategory);
            if (scanf("%d", &bedroomPick) == 0) {
                while (getchar() != '\n');
                printf("Invalid input.\n");
                bedroomPick = 0;
                continue;
            }
            while (getchar() != '\n');

            if (bedroomPick < 1 || bedroomPick > maxBedrooms)
                printf("No %s room has %d bedroom(s). Please enter between 1 and %d.\n",
                       chosenCategory, bedroomPick, maxBedrooms);

        } while (bedroomPick < 1 || bedroomPick > maxBedrooms);

        // Display: find closest match (exact or next highest)
        printf("\n--- ROOM RATES: %s, %d bedroom(s) ---\n", chosenCategory, bedroomPick);

        int found = 0;
        for (int i = 0; i < roomCount; i++) {
            if (strcmp(rooms[i].category, chosenCategory) == 0 &&
                rooms[i].bedrooms == bedroomPick) {
                printf("Room #%03d | %d bed(s) | PHP ",
                       rooms[i].roomNumber, rooms[i].bedrooms);
                inqPrintWithCommas(rooms[i].pricePerNight);
                printf("/night | %s\n", rooms[i].isAvailable ? "Vacant" : "Occupied");
                found = 1;
            }
        }

        if (!found) {
            // Show closest available bedroom count
            printf("No exact match. Showing nearest available options:\n");
            int closest = -1;
            for (int i = 0; i < roomCount; i++) {
                if (strcmp(rooms[i].category, chosenCategory) != 0) continue;
                int diff = abs(rooms[i].bedrooms - bedroomPick);
                if (closest == -1 || diff < abs(closest - bedroomPick))
                    closest = rooms[i].bedrooms;
            }
            for (int i = 0; i < roomCount; i++) {
                if (strcmp(rooms[i].category, chosenCategory) == 0 &&
                    rooms[i].bedrooms == closest) {
                    printf("Room #%03d | %d bed(s) | PHP ",
                           rooms[i].roomNumber, rooms[i].bedrooms);
                    inqPrintWithCommas(rooms[i].pricePerNight);
                    printf("/night | %s\n", rooms[i].isAvailable ? "Vacant" : "Occupied");
                }
            }
        }

        ratesAskAgain:
        printf("\nDo you have another inquiry? [y/n]: ");
        scanf("%c", &again);
        while (getchar() != '\n');
    }
}

// ─── Level 1: Amenities ───────────────────────────────────────────────────────

void inquiryAmenities() {
    char again = 'y';

    while (tolower(again) == 'y') {

        // Level 2: Pick amenity category
        printf("\n--- AMENITY CATEGORY ---\n");
        printf("  [A] - Convenience\n");
        printf("  [B] - Pool\n");
        printf("  [C] - Spa\n");
        printf("  [0] - Back\n");
        printf("Choice: ");
        char catPick;
        scanf("%c", &catPick);
        while (getchar() != '\n');
        catPick = toupper(catPick);

        if (catPick == '0') return;

        char filepath[60];
        char categoryName[20];
        switch (catPick) {
            case 'A':
                strcpy(filepath,      "Amenities/convenienceAmenite.txt");
                strcpy(categoryName,  "Convenience");
                break;
            case 'B':
                strcpy(filepath,      "Amenities/poolAmenite.txt");
                strcpy(categoryName,  "Pool");
                break;
            case 'C':
                strcpy(filepath,      "Amenities/spaAmenite.txt");
                strcpy(categoryName,  "Spa");
                break;
            default:
                printf("Invalid choice.\n");
                continue;
        }

        InqAmenity amenities[MAX_AMENITIES];
        int        amenityCount = inqReadAmenities(filepath, amenities, MAX_AMENITIES);

        if (amenityCount == 0) {
            printf("No amenities found.\n");
            goto amenitiesAskAgain;
        }

        // Level 3: Display all amenities in chosen category
        printf("\n--- %s AMENITIES ---\n", categoryName);
        printf("%-6s | %-40s | %-12s | %s\n", "Code", "Name", "Price", "Billing");
        printf("--------------------------------------------------------------------\n");
        for (int i = 0; i < amenityCount; i++) {
            printf("%-6s | %-40s | PHP ",
                   amenities[i].code, amenities[i].name);
            inqPrintWithCommas(amenities[i].price);
            printf(" | %s\n", amenities[i].type);
        }

        amenitiesAskAgain:
        printf("\nDo you have another inquiry? [y/n]: ");
        scanf("%c", &again);
        while (getchar() != '\n');
    }
}

// ─── Level 1: Room Availability ───────────────────────────────────────────────

void inquiryRoomAvailability() {
    InqRoom rooms[MAX_ROOMS];
    int     roomCount = inqReadRooms(rooms);

    if (roomCount == 0) { printf("No room data found.\n"); return; }

    char again = 'y';
    while (tolower(again) == 'y') {

        // Level 2: Pick category
        char categoryChoice = inqPickCategory("\nWhich category do you want to check?");
        if (categoryChoice == '0') return;

        const char *chosenCategory;
        switch (categoryChoice) {
            case 'A': chosenCategory = "De Luxe";      break;
            case 'B': chosenCategory = "Suite";         break;
            case 'C': chosenCategory = "Luxury Suite";  break;
            default:  printf("Invalid choice.\n"); continue;
        }

        // Level 3: Show vacant rooms in that category
        printf("\n--- AVAILABILITY: %s ---\n", chosenCategory);
        printf("%-10s | %-12s | %-17s | %s\n", "Room #", "Bedrooms", "Price/night", "Status");
        printf("------------------------------------------------------\n");

        int anyFound = 0;
        for (int i = 0; i < roomCount; i++) {
            if (strcmp(rooms[i].category, chosenCategory) == 0) {
                char bedsStr[15];
                char priceStr[20];
                sprintf(bedsStr,  "%d bed(s)", rooms[i].bedrooms);
                inqFormatPrice(rooms[i].pricePerNight, priceStr);
                printf("Room #%03d  | %-12s | PHP %-13s | %s\n",
                       rooms[i].roomNumber, bedsStr, priceStr,
                       rooms[i].isAvailable ? "VACANT" : "OCCUPIED");
                anyFound = 1;
            }
        }

        if (!anyFound)
            printf("No rooms found under %s.\n", chosenCategory);

        printf("\nDo you have another inquiry? [y/n]: ");
        scanf("%c", &again);
        while (getchar() != '\n');
    }
}

// ─── UTILS FUNCTIONS ─────────────────────────────────────────────────────────

char inqPickCategory(const char *prompt) {
    char pick;
    printf("%s\n", prompt);
    printf("  [A] - De Luxe\n");
    printf("  [B] - Suite\n");
    printf("  [C] - Luxury Suite\n");
    printf("  [0] - Back\n");
    printf("Choice: ");
    scanf("%c", &pick);
    while (getchar() != '\n');
    return toupper(pick);
}

int inqReadRooms(InqRoom *rooms) {
    FILE *file = fopen("rooms.txt", "r");
    if (!file) { printf("Could not open rooms.txt\n"); return 0; }

    char line[150];
    int  count   = 0;
    int  filled  = 0;
    InqRoom *r   = &rooms[count];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        line[strcspn(line, "\r")] = '\0';

        if (strncmp(line, "Room #", 6) == 0) {
            r = &rooms[count];
            memset(r, 0, sizeof(InqRoom));
            sscanf(line, "Room #%d:", &r->roomNumber);
            filled = 1;
        }
        else if (strncmp(line, "Category:", 9) == 0)
            sscanf(line, "Category: %29[^\r\n]", r->category);
        else if (strncmp(line, "Bedrooms:", 9) == 0)
            sscanf(line, "Bedrooms: %d", &r->bedrooms);
        else if (strncmp(line, "Price:", 6) == 0)
            sscanf(line, "Price: %f", &r->pricePerNight);
        else if (strncmp(line, "Status:", 7) == 0)
            r->isAvailable = (strstr(line, "Vacant") != NULL);

        if (line[0] == '\0' && filled) {
            count++;
            filled = 0;
        }
    }

    // catch last block if no trailing blank line
    if (filled) count++;

    fclose(file);
    return count;
}

int inqReadAmenities(const char *filename, InqAmenity *list, int maxCount) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;

    char       line[150];
    int        count       = 0;
    InqAmenity *current    = &list[count];

    while (fgets(line, sizeof(line), file) && count < maxCount) {
        line[strcspn(line, "\n")] = '\0';
        line[strcspn(line, "\r")] = '\0';

        if (strncmp(line, "Code:", 5) == 0) {
            current = &list[count];
            memset(current, 0, sizeof(InqAmenity));
            sscanf(line, "Code: %4s", current->code);
        }
        else if (strncmp(line, "Name:", 5) == 0)
            sscanf(line, "Name: %59[^\r\n]", current->name);
        else if (strncmp(line, "Price:", 6) == 0)
            sscanf(line, "Price: %f", &current->price);
        else if (strncmp(line, "Type:", 5) == 0) {
            sscanf(line, "Type: %19[^\r\n]", current->type);
            count++;
        }
    }

    fclose(file);
    return count;
}

void inqFormatPrice(float amount, char *buffer) {
    int wholeNumber = (int)amount;
    int centsPart   = (int)((amount - wholeNumber) * 100 + 0.5f);

    if (wholeNumber >= 1000000)
        sprintf(buffer, "%d,%03d,%03d.%02d",
                wholeNumber / 1000000,
                (wholeNumber / 1000) % 1000,
                wholeNumber % 1000, centsPart);
    else if (wholeNumber >= 1000)
        sprintf(buffer, "%d,%03d.%02d",
                wholeNumber / 1000,
                wholeNumber % 1000, centsPart);
    else
        sprintf(buffer, "%d.%02d", wholeNumber, centsPart);
}

void inqPrintWithCommas(float amount) {
    int wholeNumber = (int)amount;
    int centsPart   = (int)((amount - wholeNumber) * 100 + 0.5f);

    if (wholeNumber >= 1000000)
        printf("%d,%03d,%03d.%02d",
               wholeNumber / 1000000,
               (wholeNumber / 1000) % 1000,
               wholeNumber % 1000, centsPart);
    else if (wholeNumber >= 1000)
        printf("%d,%03d.%02d",
               wholeNumber / 1000,
               wholeNumber % 1000, centsPart);
    else
        printf("%d.%02d", wholeNumber, centsPart);
}