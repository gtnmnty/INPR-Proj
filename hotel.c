#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ROOMS 100
#define MAX_BOOKINGS 500
#define MAX_AMENITIES 20

#ifdef _WIN32
    #include <windows.h>
    #define DELAY(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define DELAY(ms) sleep((ms) / 1000)
#endif

char choice;

typedef struct {
  char referenceNumber[10];
  char guestName[50];
  char checkIn[20];
  char checkOut[20];
  int numberOfGuests;
  int numberOfDays;
  char roomType[30];
  int roomNumber;
  float pricePerNight;
  float roomRate;
  float amenitiesTotal;
  float finalAmount;
  float amountReceived;
  float change;
  char paymentMethod[20];
  int isPaid;
} Reservation;

typedef struct {
  int roomNumber;
  char category[30];
  int bedrooms;
  float pricePerNight;
  int isAvailable;
  char guestName[50];
  int numberOfDays;
  char checkIn[20];
  char checkOut[20];
} Room;

typedef struct {
  char code[5];
  char name[60];
  float price;
  char type[20];
} Amenity;

typedef struct {
  char referenceNumber[10];
  char roomNumber[6];
  char roomType[30];
  char guestName[50];
  char checkIn[20];
  char checkOut[20];
  int  numberOfGuests;
  int  numberOfDays;
  float pricePerNight;
  float roomRate;
  int  isPaid;
} FoundBooking;

void displayMenu();
void listVacant();
void reserve();
void bookingLookup();
void payment(const char *referenceNumber, int fromReserve);
void registry();
void viewDetails();
void inquiryMenu();
void checkout();
void exitSystem();

void generateReferenceNumber(char *output);
int readRoom(FILE *fp, Room *room);
int readAmenities(const char *filename, Amenity *list, int maxCount);
int findBooking(const char *referenceNumber, Reservation *reservation);
void updateBookingPayment(Reservation *reservation);
void printWithCommas(float amount);
char *formatPrice(float amount, char *buffer);
void receiptGenerator(Reservation *reservation, int methodPick);

int inquiryRates();
void inquiryAmenities();
void inquiryRoomAvailability();
char inqPickCategory(const char *prompt);
int  readAllRooms(Room *rooms);
int parseAmenityCodes(const char *line, char codes[][5], int maxCodes);
int findAmenityByCode(const char *code, Amenity *foundAmenity);
float calculateAmenitiesTotal(char codes[][5], int codeCount, int numberOfDays, int numberOfGuests);
int daysInMonth(int month, int year);
int monthNameToNumber(const char *name);
const char *monthNumberToName(int month);
int parseDate(const char *input, int *month, int *day, int *year);
int validateAndFormatDate(const char *input, char *output, int currentYear, int currentMonth, int currentDay);
int dateIsBefore(const char *a, const char *b);
int dateDifferenceInDays(const char *startDate, const char *endDate);

int main() {
  system("mkdir Receipts 2>nul");
  char tryAgain;
  int running = 1;

  do {
    displayMenu();

    printf("Selection: ");
    if (scanf("%c", &choice) == 0) {
      while (getchar() != '\n');
      printf("Invalid input. Please choose between the menu.\n");
      continue;
    }
    while (getchar() != '\n');

    choice = toupper(choice);

    if(choice == 'I'){
      exitSystem();

      printf("\nDo you want to go back to main menu? [y/n]: ");
      scanf(" %c", &tryAgain);
      while (getchar() != '\n');

      if (tolower(tryAgain) == 'n') {
        printf("Shutting down system...\n");
        running = 0;
      }
      continue;
    }

    switch (choice) {
      case 'A':
        listVacant();
        break;
      case 'B':
        reserve();
        break;
      case 'C':
        bookingLookup();
        break;
      case 'D': {
          char refInput[10];
          printf("Enter Reference No: ");
          scanf("%9s", refInput);
          while (getchar() != '\n');
          payment(refInput, 0);
          break;
        }
      case 'E':
        registry();
        break;
      case 'F':
        viewDetails();
        break;
      case 'G':
        inquiryMenu();
        break;
      case 'H':
        checkout();
        break;
      case 'I':
        exitSystem();
        break;
      default:
        printf("Invalid choice. Try again.\n");
    }
  } while (choice != 'I' || tolower(tryAgain) == 'y');

  return 0;
}

void displayMenu() {
  printf("\n============== ESPLENIN HOTEL ================\n");
  printf("[A]. Vacancies\n");
  printf("[B]. Reserve\n");
  printf("[C]. Booking\n");
  printf("[D]. Payment\n");
  printf("[E]. Registry\n");
  printf("[F]. Details\n");
  printf("[G]. Inquiry\n");
  printf("[H]. Checkout\n");
  printf("[I]. Exit\n");
  printf("================================================\n");
}

void listVacant() {
  FILE *file = fopen("rooms.txt", "r");
  if (!file) {
    printf("Could not open rooms.txt\n");
    return;
  }

  Room room;
  int foundAny = 0;

  printf("\n------------------------ VACANT ROOMS ------------------------\n");
  while (readRoom(file, &room)) {
    if (room.isAvailable){
      char priceStr[20];
      printf("Room #%03d | %-15s | %d bed(s) | PHP %-9s / night\n",
      room.roomNumber, room.category, room.bedrooms,
      formatPrice(room.pricePerNight, priceStr));
      foundAny = 1;
    }
  }

  if (!foundAny)
    printf("No vacant rooms currently.\n");
  fclose(file);
}

void reserve() {
  FILE *file = fopen("rooms.txt", "r");
  if (!file) { printf("Could not open rooms.txt\n"); return; }

  Room rooms[MAX_ROOMS];
  int  roomCount = 0;
  while (readRoom(file, &rooms[roomCount])) roomCount++;
  fclose(file);

  char checkIn[20], checkOut[20], guestName[50];
  int  numberOfGuests = 0, numberOfDays = 0;
  int anyMatchingVacant = 0;
  char roomTypePick;
  const char *chosenCategory;
  char payNow;

  time_t t = time(NULL);
  struct tm *now = localtime(&t);
  int currentYear = now->tm_year + 1900;
  int currentMonth = now->tm_mon + 1;
  int currentDay = now->tm_mday;

  char rawInput[50];

  do {
    do {
      printf("\nCheck-In (MM/DD/YYYY or Month DD, YYYY): ");
      fgets(rawInput, sizeof(rawInput), stdin);
      rawInput[strcspn(rawInput, "\n")] = '\0';
    } while (!validateAndFormatDate(rawInput, checkIn, currentYear, currentMonth, currentDay));

    do {
      printf("Check-Out (MM/DD/YYYY or Month DD, YYYY): ");
      fgets(rawInput, sizeof(rawInput), stdin);
      rawInput[strcspn(rawInput, "\n")] = '\0';
    } while (!validateAndFormatDate(rawInput, checkOut, currentYear, currentMonth, currentDay));

    if (!dateIsBefore(checkIn, checkOut)) {
        printf("Check-out must be after check-in (%s).\n", checkIn);
    }
  } while (!dateIsBefore(checkIn, checkOut));

  numberOfDays = dateDifferenceInDays(checkIn, checkOut);

  printf("Guest Name   : ");
  fgets(guestName, sizeof(guestName), stdin);
  guestName[strcspn(guestName, "\n")] = '\0';

  do {
    printf("No. of Guests: ");
    if (scanf("%d", &numberOfGuests) == 0) {
      while (getchar() != '\n');
      printf("Invalid input. Please enter a number.\n");
      numberOfGuests = 0;
      continue;
    }
    while (getchar() != '\n');
    
    if (numberOfGuests <= 0)
        printf("Guest count must be greater than 0.\n");
  } while (numberOfGuests <= 0);

  printf("No of Days: %d\n", numberOfDays + 1);

  do {
    printf("\nRoom Type:\n");
    printf("  [A] - Classic\n");
    printf("  [B] - De Luxe\n");
    printf("  [C] - Suite\n");
    printf("  [D] - Imperial Grand\n");
    printf("  [0] - Cancel\n");
    printf("Choice: ");
    scanf("%c", &roomTypePick);
    while (getchar() != '\n');
    roomTypePick = toupper(roomTypePick);

    if (roomTypePick == '0') {
      printf("Reservation cancelled.\n");
      return;
    }

    switch (roomTypePick) {
      case 'A': chosenCategory = "Classic";       break;
      case 'B': chosenCategory = "De Luxe";       break;
      case 'C': chosenCategory = "Suite";          break;
      case 'D': chosenCategory = "Imperial Grand"; break;
      default:  printf("Invalid choice.\n"); chosenCategory = NULL; continue;
    }

    int anyMatchingVacant = 0;
    for (int i = 0; i < roomCount; i++) {
      if (rooms[i].isAvailable &&
        strcmp(rooms[i].category, chosenCategory) == 0) {
        anyMatchingVacant = 1;
        break;
      }
    }

    if (!anyMatchingVacant) {
      printf("No vacant %s rooms available. Please choose another type or enter 0 to cancel.\n",
        chosenCategory);
      chosenCategory = NULL;
    }
  } while (chosenCategory == NULL);

  printf("\n------- AVAILABLE %s ROOMS -------\n", chosenCategory);
  for (int i = 0; i < roomCount; i++) {
    if (rooms[i].isAvailable &&
      strcmp(rooms[i].category, chosenCategory) == 0) {
      printf("Room #%03d | %d bed(s) | PHP ",
              rooms[i].roomNumber, rooms[i].bedrooms);
      printWithCommas(rooms[i].pricePerNight);
      printf(" / night\n");
      anyMatchingVacant = 1;
    }
  }

  if (!anyMatchingVacant) {
    printf("No vacant %s rooms available.\n", chosenCategory);
    return;
  }


  int selectedIndex = -1;
  do {
    int roomPick;
    printf("\nEnter preferred room number: ");
    scanf("%d", &roomPick);
    while (getchar() != '\n');

    for (int i = 0; i < roomCount; i++) {
      if (rooms[i].roomNumber == roomPick &&
        rooms[i].isAvailable  == 1 &&
        strcmp(rooms[i].category, chosenCategory) == 0) {
        selectedIndex = i;
        break;
      }
    }

    if (selectedIndex == -1)
        printf("Invalid room number. Please choose from the list above.\n");
  } while (selectedIndex == -1);

  float roomRate = rooms[selectedIndex].pricePerNight * numberOfDays;

  char referenceNumber[10];
  generateReferenceNumber(referenceNumber);

  char confirmReservation;

  printf("\nRoom Type   : %s\n", chosenCategory);
  printf("Price/night : PHP "); printWithCommas(rooms[selectedIndex].pricePerNight); printf("\n");
  printf("Room Rate   : PHP "); printWithCommas(roomRate);
  printf("\n");
  printf("Your reference no: %s\n", referenceNumber);
  printf("_____________________________________________\n");

  do{
    printf("\nDo you want to proceed? [y/n]: ");
    scanf("%c", &confirmReservation);
    while (getchar() != '\n');

    if(toupper(confirmReservation) == 'N'){
      printf("Reservation is cancelled.\n");
      return;
    }

    printf("Invalid Choice. Please Choose between the options\n");

  } while(toupper(confirmReservation) != 'Y');

  rooms[selectedIndex].isAvailable = 0;
  file = fopen("rooms.txt", "w");
  if (!file) { printf("Could not update rooms.txt\n"); return; }
  for (int i = 0; i < roomCount; i++) {
      fprintf(file, "Room #%03d:\n",  rooms[i].roomNumber);
      fprintf(file, "Category: %s\n", rooms[i].category);
      fprintf(file, "Bedrooms: %d\n", rooms[i].bedrooms);
      fprintf(file, "Price: %.2f\n",  rooms[i].pricePerNight);
      fprintf(file, "Status: %s\n",   rooms[i].isAvailable ? "Vacant" : "Occupied");
      fprintf(file, "\n");
  }
  fclose(file);

  file = fopen("bookings.txt", "a");
  if (!file) { printf("Could not open bookings.txt\n"); return; }
  fprintf(file, "\n");
  fprintf(file, "------- Guest Info -------\n");
  fprintf(file, "Reference No: %s\n",   referenceNumber);
  fprintf(file, "Room #: %03d\n",       rooms[selectedIndex].roomNumber);
  fprintf(file, "Room Type: %s\n",      chosenCategory);
  fprintf(file, "Main Guest: %s\n",     guestName);
  fprintf(file, "No of Guest: %d\n",    numberOfGuests);
  fprintf(file, "Check-In: %s\n",       checkIn);
  fprintf(file, "Checkout: %s\n",       checkOut);
  fprintf(file, "No of Days: %d\n",     numberOfDays);
  fprintf(file, "\n");
  fprintf(file, "------- Bill Info -------\n");
  fprintf(file, "Room Rate: %.2f\n",    roomRate);
  fprintf(file, "Amenities Used:\n");
  fprintf(file, "Amenities Total: 0.00\n");
  fprintf(file, "Final Amount: %.2f\n",  roomRate);
  fprintf(file, "\n");
  fprintf(file, "------- Payment Info -------\n");
  fprintf(file, "Status: Not Paid\n");
  fprintf(file, "Method:\n");
  fprintf(file, "Amount Received:\n");
  fprintf(file, "Change:\n");
  fprintf(file, "\n");
  fprintf(file, "===========================================================\n");
  fclose(file);


  printf("\nBooking\n");
  printf("Input reference no. %s\n", referenceNumber);
  printf("Details:\n");
  printf("  Check-In           :  %s\n",   checkIn);
  printf("  Check-Out          :  %s\n",   checkOut);
  printf("  Name of Main Guest :  %s\n",   guestName);
  printf("  Number of Guests   :  %d\n",   numberOfGuests);
  printf("  No. of Days        :  %d\n",   numberOfDays);
  printf("  Room Type          :  %s\n",   chosenCategory);
  printf("  Price/night        :  PHP ");  printWithCommas(rooms[selectedIndex].pricePerNight); printf("\n");
  printf("  Room Rate          :  PHP ");  printWithCommas(roomRate); printf("\n");


  do{
    printf("\nDo you want to proceed to payment? [y/n]: ");
    scanf("%c", &payNow);
    while (getchar() != '\n');

    if (tolower(payNow) == 'y')
      payment(referenceNumber, 1);
    
    if(tolower(payNow) == 'n')
      printf("Thank you for reserving. Enjoy your room.\n");
    
    if(payNow == 0) 
      printf("Thank you for reserving! You can pay later.\n");
      return;

  } while (tolower(payNow) != 'y' && tolower(payNow) != 'n' && payNow != '0');
}

void bookingLookup() {
  FILE *file;
  char searchName[50];
  char line[200];
  char anotherSearch;

  printf("\nBooking Inquiry");
  while (1) {
    printf("\nEnter guest name (or 0 to cancel): ");
    fgets(searchName, sizeof(searchName), stdin);
    searchName[strcspn(searchName, "\n")] = '\0';

    if (strcmp(searchName, "0") == 0) {
      printf("Returning to main menu.\n");
      return;
    }

    if (strlen(searchName) == 0) {
      printf("Name cannot be empty. Try again.\n");
      continue;
    }

    file = fopen("bookings.txt", "r");
    if (!file) {
        printf("Could not open bookings.txt\n");
        return;
    }

    FoundBooking found[MAX_BOOKINGS];
    int foundCount = 0;
    int insideBlock = 0;
    FoundBooking current;

    while (fgets(line, sizeof(line), file)) {
      line[strcspn(line, "\n")] = '\0';

      if (strncmp(line, "------- Guest Info", 18) == 0) {
        insideBlock = 1;
        memset(&current, 0, sizeof(FoundBooking));
        continue;
      }
      if (strncmp(line, "------- Bill Info",  17) == 0 ||
        strncmp(line, "------- Payment Info", 20) == 0) {
        insideBlock = 1;
        continue;
      }

      if (!insideBlock) continue;

      if      (strncmp(line, "Reference No:", 13) == 0)
          sscanf(line, "Reference No: %9s",       current.referenceNumber);
      else if (strncmp(line, "Room #:", 7) == 0)
          sscanf(line, "Room #: %5s",             current.roomNumber);
      else if (strncmp(line, "Room Type:", 10) == 0)
          sscanf(line, "Room Type: %29[^\n]",     current.roomType);
      else if (strncmp(line, "Main Guest:", 11) == 0)
          sscanf(line, "Main Guest: %49[^\n]",    current.guestName);
      else if (strncmp(line, "No of Guest:", 12) == 0)
          sscanf(line, "No of Guest: %d",        &current.numberOfGuests);
      else if (strncmp(line, "Check-In:", 9) == 0)
          sscanf(line, "Check-In: %19[^\n]",      current.checkIn);
      else if (strncmp(line, "Checkout:", 9) == 0)
          sscanf(line, "Checkout: %19[^\n]",      current.checkOut);
      else if (strncmp(line, "No of Days:", 11) == 0)
          sscanf(line, "No of Days: %d",         &current.numberOfDays);
      else if (strncmp(line, "Room Rate:", 10) == 0)
          sscanf(line, "Room Rate: %f",          &current.roomRate);
      else if (strncmp(line, "Status:", 7) == 0)
          current.isPaid = (strstr(line, "Not Paid") == NULL &&
                            strstr(line, "Paid") != NULL);

      if (strncmp(line, "===========", 11) == 0 && insideBlock) {
        insideBlock = 0;

        char storedLower[50], searchLower[50];
        int i;
        for (i = 0; current.guestName[i]; i++)
            storedLower[i] = tolower((unsigned char)current.guestName[i]);
        storedLower[i] = '\0';

        for (i = 0; searchName[i]; i++)
            searchLower[i] = tolower((unsigned char)searchName[i]);
        searchLower[i] = '\0';

        if (strcmp(storedLower, searchLower) == 0) {
          if (foundCount < MAX_BOOKINGS)
              found[foundCount++] = current;
        }
      }
    }
    fclose(file);

    if (foundCount == 0) {
      printf("No bookings found for \"%s\".\n", searchName);
      printf("Try another name or enter 0 to cancel.\n");
      continue; 
    }

    printf("\n");
    printf("\nBooking found for %s\n", searchName);
    for (int b = 0; b < foundCount; b++) {
      float pricePerNight = (found[b].numberOfDays > 0)
                          ? found[b].roomRate / found[b].numberOfDays : 0;

      printf("Input reference no. %s\n", found[b].referenceNumber);
      printf("Details:\n");
      printf("  Check-In           :  %s\n",   found[b].checkIn);
      printf("  Check-Out          :  %s\n",   found[b].checkOut);
      printf("  Name of Main Guest :  %s\n",   found[b].guestName);
      printf("  Number of Guests   :  %d\n",   found[b].numberOfGuests);
      printf("  No. of Days        :  %d\n",   found[b].numberOfDays);
      printf("  Room Type          :  %s\n",   found[b].roomType);
      printf("  Price/night        :  PHP ");  printWithCommas(pricePerNight); printf("\n");
      printf("  Room Rate          :  PHP ");  printWithCommas(found[b].roomRate); printf("\n");
      printf("-----------------------------------------------\n");
    }

    printf("Total booking(s) found: %d\n", foundCount);
    
    do{
      printf("\nSearch another guest? [y/n]: ");
      scanf(" %c", &anotherSearch);
      while (getchar() != '\n');

      if(tolower(anotherSearch) == 'n') return;
      if(tolower(anotherSearch) == 'y') break;

      printf("Y or N only.\n");

    } while(tolower(anotherSearch) != 'y' && tolower(anotherSearch) != 'n');
  }
}

void payment(const char *referenceNumber, int fromReserve) {
  Reservation reservation;

  if (!findBooking(referenceNumber, &reservation)) {
    printf("Booking %s not found.\n", referenceNumber);
    return;
  }

  float alreadyPaid    = reservation.amountReceived;
  float balanceRemaining = reservation.finalAmount - alreadyPaid;

  if (alreadyPaid > 0 && balanceRemaining > 0) {
    printf("\n--- BALANCE INFORMATION ---\n");
    printf("Previously Paid  : PHP "); printWithCommas(alreadyPaid);       printf("\n");
    printf("Remaining Balance: PHP "); printWithCommas(balanceRemaining);  printf("\n");
  }

  if (reservation.isPaid) {
    printf("Booking %s is already paid.\n", referenceNumber);
    return;
  }

  if (reservation.pricePerNight == 0 && reservation.numberOfDays > 0)
    reservation.pricePerNight = reservation.roomRate / reservation.numberOfDays;

  printf("\n------- Payment -------\n");
  printf("Input Reference No.  :  %s\n", reservation.referenceNumber);
  printf("Check-In             :  %s\n", reservation.checkIn);
  printf("Check-Out            :  %s\n", reservation.checkOut);
  printf("Name of Main Guest   :  %s\n", reservation.guestName);
  printf("Number of Guests     :  %d\n", reservation.numberOfGuests);
  printf("No. of Days          :  %d\n", reservation.numberOfDays);
  printf("Room Type            :  %s\n", reservation.roomType);
  printf("Price/night          :  PHP "); printWithCommas(reservation.pricePerNight); printf("\n");
  printf("Room Rate            :  PHP "); printWithCommas(reservation.roomRate);      printf("\n");

  if (fromReserve) {
      Amenity selectedAmenities[MAX_AMENITIES];
      int selectedCount = 0;
      char addMore = 'y';

      do{
        printf("\nWould you like to add Amenities? [y/n]: ");
        scanf("%c", &addMore);
        while (getchar() != '\n');

        addMore = tolower(addMore);

        if(addMore != 'y' || addMore !='n'){
          printf("Y or N only.\n");
        }
      } while(addMore != 'y' && addMore != 'n');
      
      while (tolower(addMore) == 'y') {
        printf("\n----- AMENITIES -----\n");
        printf("  [A] - Convenience\n");
        printf("  [B] - Pool\n");
        printf("  [C] - Spa\n");
        printf("  [N] - No Amenities\n");
        printf("Choice: ");

        char categoryPick;
        scanf("%c", &categoryPick);
        while (getchar() != '\n');
        categoryPick = toupper(categoryPick);

        if (categoryPick == 'N') break;

        char filepath[60];
        switch (categoryPick) {
            case 'A': strcpy(filepath, "Amenities/convenienceAmenite.txt"); break;
            case 'B': strcpy(filepath, "Amenities/poolAmenite.txt");        break;
            case 'C': strcpy(filepath, "Amenities/spaAmenite.txt");         break;
            default:  printf("Invalid choice.\n"); continue;
        }

        Amenity availableAmenities[10];
        int availableCount = readAmenities(filepath, availableAmenities, 10);

        if (availableCount == 0) { printf("No amenities found in file.\n"); continue; }

        char priceBuffer[20];
        printf("\n%-6s | %-20s | %-16s | %s\n", "Code", "Name", "Price", "Type");
        printf("-------------------------------------------------------------------------------------------\n");
        for (int i = 0; i < availableCount; i++) {
            printf("%-6s | %-20s | PHP %-12s | %-10s\n",
                availableAmenities[i].code,
                availableAmenities[i].name,
                formatPrice(availableAmenities[i].price, priceBuffer),
                availableAmenities[i].type);
        }

        int foundIndex = -1;
        while (1) {
          printf("\nEnter code to add (or 0 to skip): ");
          char codePick[5];
          scanf("%4s", codePick);
          while (getchar() != '\n');

          if (strcmp(codePick, "0") == 0) break;

          for (int i = 0; i < availableCount; i++) {
            if (strcasecmp(availableAmenities[i].code, codePick) == 0) {
              foundIndex = i;
              break;
            }
          }

          if (foundIndex == -1) {
            printf("Code not found. Please choose from the available codes:\n");
            for (int i = 0; i < availableCount; i++)
                printf("  [%s] %s\n", availableAmenities[i].code, availableAmenities[i].name);
            continue;
          }

          break;
        }

        if (foundIndex != -1) {
          float amenityCost = (strcmp(availableAmenities[foundIndex].type, "PerNight") == 0)
              ? availableAmenities[foundIndex].price * (reservation.numberOfDays - 1)
              : availableAmenities[foundIndex].price * reservation.numberOfGuests;

          reservation.amenitiesTotal += amenityCost;
          if (selectedCount < MAX_AMENITIES)
              selectedAmenities[selectedCount++] = availableAmenities[foundIndex];

          printf("Added: %s -- PHP ", availableAmenities[foundIndex].name);
          printWithCommas(amenityCost);
          printf("\n");
        }

        do {
          printf("\nAdd another amenity? [y/n]: ");
          scanf(" %c", &addMore);
          while (getchar() != '\n');

          addMore = tolower(addMore);

          if (addMore != 'y' && addMore != 'n')
            printf("Y or N only.\n");

        } while (addMore != 'y' && addMore != 'n');
      }

      if (selectedCount > 0) {
        FILE *file = fopen("bookings.txt", "r");
        if (file) {
          char lines[MAX_BOOKINGS][200];
          int  totalLines = 0;
          while (totalLines < MAX_BOOKINGS &&
                  fgets(lines[totalLines], sizeof(lines[totalLines]), file)) {
            lines[totalLines][strcspn(lines[totalLines], "\n")] = '\0';
            totalLines++;
          }
          fclose(file);

          int inTargetBlock = 0;
          for (int i = 0; i < totalLines; i++) {
            if (strncmp(lines[i], "Reference No:", 13) == 0) {
              char tmp[10];
              sscanf(lines[i], "Reference No: %9s", tmp);
              inTargetBlock = (strcmp(tmp, reservation.referenceNumber) == 0);
            }
            if (inTargetBlock && strncmp(lines[i], "Amenities Used:", 15) == 0) {
              char amenityLine[200] = "Amenities Used:";
              for (int j = 0; j < selectedCount; j++) {
                strcat(amenityLine, " ");
                strcat(amenityLine, selectedAmenities[j].code);
                if (j < selectedCount - 1) strcat(amenityLine, ",");
              }
              strcpy(lines[i], amenityLine);
            }
            if (strncmp(lines[i], "===========", 11) == 0)
              inTargetBlock = 0;
          }

          file = fopen("bookings.txt", "w");
          if (file) {
            for (int i = 0; i < totalLines; i++)
              fprintf(file, "%s\n", lines[i]);
            fclose(file);
          }
        }
      }
  }

  reservation.finalAmount = reservation.roomRate + reservation.amenitiesTotal;


  printf("\n========================================\n");
  printf("           BILL BREAKDOWN\n");
  printf("========================================\n");
  printf("Reference No  : %s\n", reservation.referenceNumber);
  printf("Main Guest    : %s\n", reservation.guestName);
  printf("No. of Days   : %d\n", reservation.numberOfDays);
  printf("Room Type     : %s\n", reservation.roomType);
  printf("----------------------------------------\n");
  printf("Room Rate     : PHP "); printWithCommas(reservation.roomRate);       printf("\n");
  printf("Amenities     : PHP "); printWithCommas(reservation.amenitiesTotal); printf("\n");
  printf("----------------------------------------\n");
  printf("FINAL AMOUNT  : PHP "); printWithCommas(reservation.finalAmount);    printf("\n");
  printf("========================================\n");


  int methodPick = 0;
    do {
      printf("\nPayment Method:\n");
      printf("  [1] - Cash\n");
      printf("  [2] - Card\n");
      printf("  [3] - GCash\n");
      printf("Choice: ");

      if (scanf("%d", &methodPick) == 0) {
        while (getchar() != '\n');
        printf("Invalid input. Please enter 1, 2, or 3.\n");
        methodPick = 0;
      } else {
        while (getchar() != '\n');
        if (methodPick < 1 || methodPick > 3) {
          printf("Invalid choice. Please select 1, 2, or 3.\n");
          methodPick = 0;
        }
      }
    } while (methodPick == 0);

  if (methodPick == 1) {
      strcpy(reservation.paymentMethod, "Cash");

      do {
          printf("Amount Received: PHP ");
          if (scanf("%f", &reservation.amountReceived) == 0) {
            while (getchar() != '\n');
            printf("Invalid input. Please enter a valid amount.\n");
            reservation.amountReceived = 0;
            continue;
          }
          while (getchar() != '\n');

          if (reservation.amountReceived <= 0)
              printf("Amount must be greater than 0. Please try again.\n");
          else if (reservation.amountReceived < reservation.finalAmount) {
              char buf[20];
              printf("Insufficient amount. Total due is PHP %s. Please try again.\n",
                formatPrice(reservation.finalAmount, buf));
          }
      } while (reservation.amountReceived <= 0 ||
          reservation.amountReceived < reservation.finalAmount);

      reservation.change = reservation.amountReceived - reservation.finalAmount;

  } else if (methodPick == 2) {
      strcpy(reservation.paymentMethod, "Card");

      char cardHolderName[20], cardNumber[20], expiryDate[8], cvv[5];
      int valid = 0, expvalid = 0;

      printf("\n--- Card Details ---\n");

      printf("Cardholder Name    : ");
      fgets(cardHolderName, sizeof(cardHolderName), stdin);
      cardHolderName[strcspn(cardHolderName, "\n")] = '\0';

      while (!valid) {
        printf("Card Number (12-16 digits): ");
        scanf("%s", cardNumber);

        int len = strlen(cardNumber);
        int isNumeric = 1;

        for (int i = 0; i < len; i++) {
          if (!isdigit(cardNumber[i])) {
            isNumeric = 0;
            break;
          }
        }

        if (!isNumeric) {
          printf("Invalid input. Card number cannot be negative or contain letters.\n");
        } else if (len < 12 || len > 16) {
          printf("Invalid length. Please enter 12 to 16 digits.\n");
        } else {
          valid = 1;
        }
      }

      while (!expvalid) {
        printf("Expiry Date (MM/YY): ");
        scanf("%s", expiryDate);
        int len = strlen(expiryDate);
        if (len == 5 && expiryDate[2] == '/') {
            expvalid = 1;
        } else {
            printf("Invalid format. Please enter in MM/YY format (e.g. 08/27).\n");
        }
      }

      do {
        printf("CVV (3 digits)     : ");
        scanf("%4s", cvv);
        while (getchar() != '\n');
        if (strlen(cvv) != 3)
          printf("Invalid CVV. Must be exactly 3 digits.\n");
      } while (strlen(cvv) != 3);

      reservation.amountReceived = reservation.finalAmount;
      reservation.change         = 0;
      printf("Processing card payment...\n");

  } else {

      strcpy(reservation.paymentMethod, "GCash");

      char gcashNumber[15];
      char gcashName[50];

      printf("\n--- GCash Details ---\n");

      do {
        printf("GCash Number (11 digits): ");
        scanf("%14s", gcashNumber);
        while (getchar() != '\n');

        int allDigits = 1;
        for (int i = 0; i < (int)strlen(gcashNumber); i++) {
          if (!isdigit(gcashNumber[i])) { allDigits = 0; break; }
        }

        if (!allDigits || strlen(gcashNumber) != 11)
          printf("Invalid number. Must be exactly 11 digits.\n");
        else
          break;
      } while (1);

      do {
        printf("GCash Account Name   : ");
        fgets(gcashName, sizeof(gcashName), stdin);
        gcashName[strcspn(gcashName, "\n")] = '\0';

        int hasDigit = 0;
        for (int i = 0; i < (int)strlen(gcashName); i++) {
          if (isdigit(gcashName[i])) { hasDigit = 1; break; }
        }

        if (hasDigit) {
          printf("Invalid name. Name must not contain numbers.\n");
          continue;
        }
        if (strlen(gcashName) < 2) {
          printf("Name too short. Please enter your full name.\n");
          continue;
        }
        break;
      } while (1);

      reservation.amountReceived = reservation.finalAmount;
      reservation.change         = 0;
      printf("Processing GCash payment...\n");
  }


  reservation.isPaid = 1;

  printf("\n========================================\n");
  printf("         PAYMENT SUCCESSFUL\n");
  printf("========================================\n");
  printf("Reference No    : %s\n", reservation.referenceNumber);
  printf("Guest           : %s\n", reservation.guestName);
  printf("Payment Method  : %s\n", reservation.paymentMethod);
  printf("Total Paid      : PHP "); printWithCommas(reservation.finalAmount);    printf("\n");
  if (methodPick == 1) {
      printf("Amount Received : PHP "); printWithCommas(reservation.amountReceived); printf("\n");
      printf("Change          : PHP "); printWithCommas(reservation.change);         printf("\n");
  }
  printf("========================================\n");
  printf("Thank you, %s! Enjoy your room! :).\n", reservation.guestName);

  updateBookingPayment(&reservation);
  receiptGenerator(&reservation, methodPick);
}

int isLeapYear(int year) {
  return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int daysInMonth(int month, int year) {
  int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 2 && isLeapYear(year))
      return 29;
  return days[month];
}

int monthNameToNumber(const char *name) {
  const char *months[] = {
      "january","february","march","april","may","june",
      "july","august","september","october","november","december"
  };
  char lower[20] = "";
  for (int i = 0; name[i] && i < 19; i++)
      lower[i] = tolower((unsigned char)name[i]);

  for (int i = 0; i < 12; i++)
      if (strcmp(lower, months[i]) == 0) return i + 1;
  return -1;
}

const char *monthNumberToName(int month) {
  const char *names[] = {
    "","January","February","March","April","May","June",
    "July","August","September","October","November","December"
  };
  if (month < 1 || month > 12) return "";
  return names[month];
}

int parseDate(const char *input, int *month, int *day, int *year) {
  if (sscanf(input, "%d/%d/%d", month, day, year) == 3)
      return 1;

  char monthStr[20] = "";
  if (sscanf(input, "%19s %d, %d", monthStr, day, year) == 3) {
    monthStr[strcspn(monthStr, ",")] = '\0';
    *month = monthNameToNumber(monthStr);
    if (*month != -1) return 1;
  }

  if (sscanf(input, "%19s %d %d", monthStr, day, year) == 3) {
    *month = monthNameToNumber(monthStr);
    if (*month != -1) return 1;
  }
  return 0;
}

long dateSerial(int month, int day, int year) {
  long serial = 365L * (year - 1) + (year - 1) / 4 - (year - 1) / 100 + (year - 1) / 400;

  for (int m = 1; m < month; m++)
    serial += daysInMonth(m, year);

  return serial + day;
}

int validateAndFormatDate(const char *input, char *output, int currentYear, int currentMonth, int currentDay) {
  int month = 0, day = 0, year = 0;

  if (!parseDate(input, &month, &day, &year)) {
    printf("Invalid format. Use MM/DD/YYYY or Month DD, YYYY (e.g. 03/12/2026 or March 12, 2026).\n");
    return 0;
  }

  if (month < 1 || month > 12) {
    printf("Invalid month. Must be 1-12.\n");
    return 0;
  }

  if (year < currentYear) {
    printf("Invalid year. Must be %d or later.\n", currentYear);
    return 0;
  }

  int maxDay = daysInMonth(month, year);
  if (day < 1 || day > maxDay) {
    printf("Invalid day. %s %d has %d days.\n",
            monthNumberToName(month), year, maxDay);
    return 0;
}

  if (dateSerial(month, day, year) < dateSerial(currentMonth, currentDay, currentYear)) {
    printf("Invalid date. Reservation date cannot be before today.\n");
    return 0;
  }

  sprintf(output, "%s %d, %d", monthNumberToName(month), day, year);
  return 1;
}

int dateIsBefore(const char *a, const char *b) {
  int mA, dA, yA, mB, dB, yB;
  parseDate(a, &mA, &dA, &yA);
  parseDate(b, &mB, &dB, &yB);

  if (yA != yB) return yA < yB;
  if (mA != mB) return mA < mB;
  return dA < dB;
}

int dateDifferenceInDays(const char *startDate, const char *endDate) {
  int startMonth = 0, startDay = 0, startYear = 0;
  int endMonth = 0, endDay = 0, endYear = 0;

  if (!parseDate(startDate, &startMonth, &startDay, &startYear) ||
      !parseDate(endDate, &endMonth, &endDay, &endYear))
    return 0;

  return (int)(dateSerial(endMonth, endDay, endYear) -
               dateSerial(startMonth, startDay, startYear));
}

void registry() {
  char searchName[50];
  char searchAgain;
  char actionChoice;
  char confirm;

  printf("\n----------- REGISTRY -----------\n");
  do {
    printf("Enter full name to search: ");
    fgets(searchName, sizeof(searchName), stdin);
    searchName[strcspn(searchName, "\n")] = '\0';

    FILE *file = fopen("bookings.txt", "r");
    if (!file) { printf("Could not open bookings.txt\n"); return; }

    FoundBooking results[MAX_BOOKINGS];
    int resultCount = 0;
    char line[200];
    int  insideBlock = 0;

    char  tempRef[10]   = "";
    char  tempRoom[6]   = "";
    char  tempType[30]  = "";
    char  tempName[50]  = "";
    char  tempIn[20]    = "";
    char  tempOut[20]   = "";
    int   tempGuests    = 0;
    int   tempDays      = 0;
    float tempRate      = 0;
    int   tempPaid      = 0;

    while (fgets(line, sizeof(line), file)) {
      line[strcspn(line, "\n")] = '\0';

      if (strncmp(line, "------- Guest Info -------", 18) == 0) {
        insideBlock = 1;
        tempRef[0] = tempRoom[0] = tempType[0] = '\0';
        tempName[0] = tempIn[0] = tempOut[0] = '\0';
        tempGuests = tempDays = tempPaid = 0;
        tempRate = 0;
      }

      if (!insideBlock) continue;

      if (strncmp(line, "Reference No:", 13) == 0)
          sscanf(line, "Reference No: %9s",    tempRef);
      else if (strncmp(line, "Room #:", 7) == 0)
          sscanf(line, "Room #: %5s",           tempRoom);
      else if (strncmp(line, "Room Type:", 10) == 0)
          sscanf(line, "Room Type: %29[^\n]",   tempType);
      else if (strncmp(line, "Main Guest:", 11) == 0)
          sscanf(line, "Main Guest: %49[^\n]",  tempName);
      else if (strncmp(line, "Check-In:", 9) == 0)
          sscanf(line, "Check-In: %19[^\n]",    tempIn);
      else if (strncmp(line, "Checkout:", 9) == 0)
          sscanf(line, "Checkout: %19[^\n]",    tempOut);
      else if (strncmp(line, "No of Guest:", 12) == 0)
          sscanf(line, "No of Guest: %d",       &tempGuests);
      else if (strncmp(line, "No of Days:", 11) == 0)
          sscanf(line, "No of Days: %d",        &tempDays);
      else if (strncmp(line, "Room Rate:", 10) == 0)
          sscanf(line, "Room Rate: %f",          &tempRate);
      else if (strncmp(line, "Status:", 7) == 0)
          tempPaid = (strstr(line, "Not Paid") == NULL && strstr(line, "Paid") != NULL);

      if (strncmp(line, "===========", 11) == 0 && insideBlock) {
        insideBlock = 0;
        if (strcasecmp(tempName, searchName) == 0 && resultCount < MAX_BOOKINGS) {
          strcpy(results[resultCount].referenceNumber, tempRef);
          strcpy(results[resultCount].roomNumber,      tempRoom);
          strcpy(results[resultCount].roomType,        tempType);
          strcpy(results[resultCount].guestName,       tempName);
          strcpy(results[resultCount].checkIn,         tempIn);
          strcpy(results[resultCount].checkOut,        tempOut);
          resultCount++;
        }
      }
    }
    fclose(file);

    if (resultCount == 0) {
      printf("No bookings found for \"%s\".\n", searchName);
    } else {
      printf("\n------- BOOKING FOUND -------\n");
      for (int i = 0; i < resultCount; i++) {
        printf("Reference No    : %s\n",  results[i].referenceNumber);
        printf("Room #          : %s\n",  results[i].roomNumber);
        printf("Room Type       : %s\n",  results[i].roomType);
        printf("Main Guest      : %s\n",  results[i].guestName);
        if (i < resultCount - 1)
          printf("-----------------------------\n");
      }

      while (1) {
        printf("\nWhat would you like to do?\n");
        printf("  [C] - Cancel Reservation\n");
        printf("  [N] - Nothing, go back\n");
        printf("Choice: ");
        scanf(" %c", &actionChoice);
        while (getchar() != '\n');
        actionChoice = toupper(actionChoice);

        if (actionChoice == 'C' || actionChoice == 'N') break;
        printf("Invalid choice. Please enter C, or N.\n");
      }

      char targetRef[10];
      Reservation res;

      if (actionChoice == 'N') return;

      if (actionChoice == 'C' || actionChoice == 'c') {
        printf("\nEnter reference number to cancel [0 to skip]: ");
        scanf("%9s", targetRef);
        while (getchar() != '\n');

        if (strcmp(targetRef, "0") == 0)
          goto search_again;

        int isValid = 1;
        int len = strlen(targetRef);

        if (len < 2) {
          isValid = 0;
        } else {
          for (int i = 0; targetRef[i] != '\0'; i++) {
            if (!isalnum((unsigned char)targetRef[i])) {
              isValid = 0;
              break;
            }
          }
        }

        if (!isValid) {
          printf("[ERROR] Please enter a valid reference (e.g. B0005).\n");
          continue;
        }

        if (!findBooking(targetRef, &res)) {
          printf("[ERROR] Reference number %s not found.\n", targetRef);
          continue;
        }
      }

      printf("\n--- CANCELLING RESERVATION ---\n");
      printf("Reference No : %s\n", res.referenceNumber);
      printf("Guest        : %s\n", res.guestName);
      printf("Room Type    : %s\n", res.roomType);
      printf("Check-In     : %s\n", res.checkIn);
      printf("Check-Out    : %s\n", res.checkOut);

      do{
        printf("\nAre you sure? [y/n]: ");
        scanf(" %c", &confirm);
        while (getchar() != '\n');

        if(tolower(confirm) == 'n') 
          printf("Cancellation aborted.\n");
          return;

        if(tolower(confirm) == 'y') break;

        printf("Y or N only.\n");

      } while(tolower(confirm) != 'y');

      FILE *bFile = fopen("bookings.txt", "r");
      if (!bFile) { printf("Could not open bookings.txt\n"); return; }

      char lines[MAX_BOOKINGS][200];
      int  totalLines = 0;
      while (totalLines < MAX_BOOKINGS &&
             fgets(lines[totalLines], sizeof(lines[totalLines]), bFile)) {
        lines[totalLines][strcspn(lines[totalLines], "\n")] = '\0';
        totalLines++;
      }
      fclose(bFile);

      int deleteStart = -1, deleteEnd = -1, inTarget = 0;
      for (int i = 0; i < totalLines; i++) {
        if (strncmp(lines[i], "------- Guest Info", 18) == 0)
          inTarget = 1;
        if (inTarget && strncmp(lines[i], "Reference No:", 13) == 0) {
          char tmp[10];
          sscanf(lines[i], "Reference No: %9s", tmp);
          if (strcmp(tmp, targetRef) == 0)
            deleteStart = i - 1;
        }
        if (inTarget && strncmp(lines[i], "===========", 11) == 0) {
          if (deleteStart != -1) { deleteEnd = i; break; }
          inTarget = 0;
        }
      }

      bFile = fopen("bookings.txt", "w");
      if (!bFile) { printf("Could not update bookings.txt\n"); return; }
      for (int i = 0; i < totalLines; i++) {
        if (deleteStart != -1 && i >= deleteStart && i <= deleteEnd)
          continue;
        fprintf(bFile, "%s\n", lines[i]);
      }
      fclose(bFile);

      FILE *roomFile = fopen("rooms.txt", "r");
      Room rooms[MAX_ROOMS];
      int  roomCount = 0;
      while (readRoom(roomFile, &rooms[roomCount])) roomCount++;
      fclose(roomFile);

      for (int i = 0; i < roomCount; i++) {
        if (rooms[i].roomNumber == res.roomNumber) {
          rooms[i].isAvailable = 1;
          break;
        }
      }

      roomFile = fopen("rooms.txt", "w");
      for (int i = 0; i < roomCount; i++) {
        fprintf(roomFile, "Room #%03d:\n",  rooms[i].roomNumber);
        fprintf(roomFile, "Category: %s\n", rooms[i].category);
        fprintf(roomFile, "Bedrooms: %d\n", rooms[i].bedrooms);
        fprintf(roomFile, "Price: %.2f\n",  rooms[i].pricePerNight);
        fprintf(roomFile, "Status: %s\n",   rooms[i].isAvailable ? "Vacant" : "Occupied");
        fprintf(roomFile, "\n");
      }
      fclose(roomFile);

      printf("[SUCCESS] Reservation %s has been cancelled.\n", targetRef);
      printf("------------------------------------------\n");
    }

    goto search_again;

    search_again:
    do{
      printf("\nSearch another guest? [y/n]: ");
      scanf(" %c", &searchAgain);
      while (getchar() != '\n');

      if(tolower(searchAgain) == 'n') return;
      if(tolower(searchAgain) == 'y') break;

      printf("Y or N only.\n");

    } while(tolower(searchAgain) != 'y' && tolower(searchAgain) != 'y');

  } while (tolower(searchAgain) == 'y');

  printf("Returning to main menu.\n");
}

void viewDetails() {
  FILE *file = fopen("rooms.txt", "r");
  if (!file) {
    printf("Could not open rooms.txt\n");
    return;
  }

  Room rooms[MAX_ROOMS];
  int roomCount = 0;
  while (readRoom(file, &rooms[roomCount]))
    roomCount++;
  fclose(file);

  printf("\n--- ALL ROOMS ---\n");
  for (int i = 0; i < roomCount; i++) {
    printf("Room #%03d | %-20s\n", rooms[i].roomNumber, rooms[i].category);
  }

  int roomPick;
  char inputBuf[32];
  int validInput;

  do {
    do {
      printf("\nEnter room number to view: ");
      fgets(inputBuf, sizeof(inputBuf), stdin);
      inputBuf[strcspn(inputBuf, "\r\n")] = '\0';

      validInput = 1;
      for (int i = 0; inputBuf[i]; i++) {
        if (!isdigit((unsigned char)inputBuf[i])) {
          validInput = 0;
          break;
        }
      }

      if (!validInput || strlen(inputBuf) == 0) {
        printf("Invalid input. Numbers only.\n");
        continue;
      }

      roomPick = atoi(inputBuf);

      if (roomPick <= 0) {
        printf("Invalid input. Enter a positive room number.\n");
        validInput = 0;
        continue;
      }
    } while (!validInput);


    int selectedIndex = -1;
    for (int i = 0; i < roomCount; i++) {
      if (rooms[i].roomNumber == roomPick) {
        selectedIndex = i;
        break;
      }
    }

    if (selectedIndex == -1) {
      printf("Room #%03d not found.\n", roomPick);
    } else {
      Room selected = rooms[selectedIndex];
      printf("\n--- ROOM DETAILS ---\n");
      printf("Room     : #%03d\n",   selected.roomNumber);
      printf("Category : %s\n",      selected.category);
      printf("Bedrooms : %d\n",      selected.bedrooms);
      printf("Price    : PHP ");
      printWithCommas(selected.pricePerNight);
      printf("/night\n");
      printf("Status   : %s\n",      selected.isAvailable ? "Vacant" : "Occupied");
    }


    char again;
    do {
      printf("\nView another room? [y/n]: ");
      scanf(" %c", &again);
      while (getchar() != '\n');

      if (toupper(again) == 'N') return;
      if (toupper(again) == 'Y') break;

      printf("Invalid choice. Enter Y or N only.\n");
    } while (1);

  } while (1);
}

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
    if (scanf("%d", &pick) != 1) {
      while (getchar() != '\n');
      continue;
    }

    switch (pick) {
    case 1:
      if (inquiryRates() == 0) pick = 0;
      break;
    case 2:
      inquiryAmenities();
      break;
    case 3:
      inquiryRoomAvailability();
      break;
    case 0:
      printf("Thank you! Have a nice day.\n");
      break;
    default:
      printf("Invalid choice.\n");
    }

  } while (pick != 0);
}

int inquiryRates() {
  Room rooms[MAX_ROOMS];
  int roomCount = readAllRooms(rooms);

  if (roomCount == 0) {
    printf("No room data found.\n");
    return 0;
  }

  char again;

  while (1) {
    char categoryChoice = inqPickCategory("\nWhich room category are you interested in?");
    if (categoryChoice == '0')
      return 0;

    const char *chosenCategory;
    switch (categoryChoice) {
    case 'A':
      chosenCategory = "Classic"; break;
    case 'B':
      chosenCategory = "De Luxe"; break;
    case 'C':
      chosenCategory = "Suite"; break;
    case 'D':
      chosenCategory = "Imperial Grand"; break;
    default:
      printf("Invalid choice.\n");
      continue;
    }


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

    int bedroomPick = 0;
    do {
      printf("\nHow many bedrooms? (1 - %d, max for %s): ", maxBedrooms, chosenCategory);
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


    printf("\n------ ROOM RATES: %s, %d bedroom(s) ------\n", chosenCategory, bedroomPick);

    int found = 0;
    for (int i = 0; i < roomCount; i++) {
      if (strcmp(rooms[i].category, chosenCategory) == 0 &&
          rooms[i].bedrooms == bedroomPick) {
        printf("\nRoom #%03d | %d bed(s) | PHP ",
               rooms[i].roomNumber, rooms[i].bedrooms);
        printWithCommas(rooms[i].pricePerNight);
        printf("/night | %s\n", rooms[i].isAvailable ? "Vacant" : "Occupied");
        found = 1;
      }
    }

    if (!found) {

      printf("No exact match. Showing nearest available options:\n");
      int closest = -1;
      for (int i = 0; i < roomCount; i++) {
        if (strcmp(rooms[i].category, chosenCategory) != 0)
          continue;
        int diff = abs(rooms[i].bedrooms - bedroomPick);
        if (closest == -1 || diff < abs(closest - bedroomPick))
          closest = rooms[i].bedrooms;
      }
      for (int i = 0; i < roomCount; i++) {
        if (strcmp(rooms[i].category, chosenCategory) == 0 &&
            rooms[i].bedrooms == closest) {
          printf("Room #%03d | %d bed(s) | PHP ",
                 rooms[i].roomNumber, rooms[i].bedrooms);
          printWithCommas(rooms[i].pricePerNight);
          printf("/night | %s\n", rooms[i].isAvailable ? "Vacant" : "Occupied");
        }
      }
    }

    ratesAskAgain:
    do{
      printf("\nDo you have another inquiry? [y/n]: ");
      scanf(" %c", &again);
      while (getchar() != '\n');

      if(tolower(again) == 'n') return 0;
      if(tolower(again) == 'y') return 1;

    } while(1); 
  }
}

void inquiryAmenities() {
  char again = 'y';

  while (tolower(again) == 'y') {

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

    if (catPick == '0')
      return;

    char filepath[60];
    char categoryName[20];
    switch (catPick){
    case 'A':
      strcpy(filepath, "Amenities/convenienceAmenite.txt");
      strcpy(categoryName, "Convenience");
      break;
    case 'B':
      strcpy(filepath, "Amenities/poolAmenite.txt");
      strcpy(categoryName, "Pool");
      break;
    case 'C':
      strcpy(filepath, "Amenities/spaAmenite.txt");
      strcpy(categoryName, "Spa");
      break;
    default:
      printf("Invalid choice.\n");
      continue;
    }

    Amenity amenities[MAX_AMENITIES];
    int amenityCount = readAmenities(filepath, amenities, MAX_AMENITIES);

    if (amenityCount == 0){
      printf("No amenities found.\n");
      goto amenitiesAskAgain;
    }


    printf("\n------------------- %s AMENITIES -------------------\n", categoryName);
    printf("%-6s | %-20s | %-15s | %s\n", "Code", "Name", "Price", "Billing");
    printf("-------------------------------------------------------------\n");
    for (int i = 0; i < amenityCount; i++){
      char priceStr[20];
      printf("%-6s | %-20s | PHP %-11s | %s\n",
       amenities[i].code, amenities[i].name,
       formatPrice(amenities[i].price, priceStr),
       amenities[i].type);
    }

  amenitiesAskAgain:
    printf("\nDo you have another inquiry? [y/n]: ");
    scanf("%c", &again);
    while (getchar() != '\n');
  }
}

void inquiryRoomAvailability(){
  Room rooms[MAX_ROOMS];
  int roomCount = readAllRooms(rooms);

  if (roomCount == 0) {
    printf("No room data found.\n");
    return;
  }

  char again = 'y';
  while (tolower(again) == 'y'){


    char categoryChoice = inqPickCategory("\nWhich category do you want to check?");
    if (categoryChoice == '0')
      return;

    const char *chosenCategory;
    switch (categoryChoice) {
    case 'A':
      chosenCategory = "Classic";
      break;
    case 'B':
      chosenCategory = "De Luxe";
      break;
    case 'C':
      chosenCategory = "Suite";
      break;
    case 'D':
      chosenCategory = "Imperial Grand";
      break;
    default:
      printf("Invalid choice.\n");
      continue;
    }

    int anyFound = 0;
    for(int i = 0; i < roomCount; i++){
      if(strcmp(rooms[i].category, chosenCategory) == 0){ anyFound = 1; break; }
    }

    if (!anyFound){
      printf("No rooms found under %s.\n", chosenCategory);
    }
    else{
      printf("\n---------------- AVAILABILITY: %s ----------------\n", chosenCategory);
      printf("%-10s | %-12s | %-15s | %s\n", "Room #", "Bedrooms", "Price/night", "Status");
      printf("------------------------------------------------------\n");

      for (int i = 0; i < roomCount; i++) {
        if (strcmp(rooms[i].category, chosenCategory) == 0) {
          char bedsStr[15];
          char priceStr[20];

          sprintf(bedsStr, "%d bed(s)", rooms[i].bedrooms);
          formatPrice(rooms[i].pricePerNight, priceStr);
          printf("Room #%03d  | %-12s | PHP %-11s | %s\n",
                rooms[i].roomNumber, bedsStr, priceStr,
                rooms[i].isAvailable ? "VACANT" : "OCCUPIED");
          anyFound = 1;
        }
      }
    }

    printf("\nDo you have another inquiry? [y/n]: ");
    scanf("%c", &again);
    while (getchar() != '\n');
  }
}

void checkout() {
  printf("\n========================================\n");
  printf("           ESPLENIN - CHECKOUT\n");
  printf("========================================\n");

  char referenceInput[10];

  while (1) {
    printf("\nEnter Reference Number (0 to cancel): ");
    scanf("%9s", referenceInput);
    while (getchar() != '\n');

    if (strcmp(referenceInput, "0") == 0) {
      printf("Returning to main menu.\n");
      return;
    }


    Reservation reservation;
    if (!findBooking(referenceInput, &reservation)) {
      printf("Invalid reference number. Please try again.\n");
      continue;
    }


    printf("\n--- GUEST DETAILS ---\n");
    printf("Guest Name : %s\n",   reservation.guestName);
    printf("Room No.   : %03d (%s)\n", reservation.roomNumber, reservation.roomType);
    printf("Total Paid : PHP ");  printWithCommas(reservation.finalAmount); printf("\n");
    printf("Status     : %s\n",   reservation.isPaid ? "PAID" : "NOT PAID");

    if (!reservation.isPaid) {
      printf("\nWarning: This booking has not been paid yet.\n");
      printf("Please settle payment before checking out.\n");
      payment(referenceInput, 0);
    }


    printf("\nAre you sure you want to check out this guest? [y/n]: ");
    char confirm;
    scanf("%c", &confirm);
    while (getchar() != '\n');

    if (tolower(confirm) != 'y') {
      printf("Checkout cancelled. Returning to main menu.\n");
      return;
    }


    printf("\nUpdating room status...\n");
    DELAY(1500);

    FILE *roomFile = fopen("rooms.txt", "r");
    if (!roomFile) { printf("Could not open rooms.txt\n"); return; }

    Room rooms[MAX_ROOMS];
    int  roomCount = 0;
    while (readRoom(roomFile, &rooms[roomCount])) roomCount++;
    fclose(roomFile);

    for (int i = 0; i < roomCount; i++) {
      if (rooms[i].roomNumber == reservation.roomNumber) {
        rooms[i].isAvailable = 1;
        break;
      }
    }

    roomFile = fopen("rooms.txt", "w");
    if (!roomFile) { printf("Could not update rooms.txt\n"); return; }
    for (int i = 0; i < roomCount; i++) {
      fprintf(roomFile, "Room #%03d:\n",  rooms[i].roomNumber);
      fprintf(roomFile, "Category: %s\n", rooms[i].category);
      fprintf(roomFile, "Bedrooms: %d\n", rooms[i].bedrooms);
      fprintf(roomFile, "Price: %.2f\n",  rooms[i].pricePerNight);
      fprintf(roomFile, "Status: %s\n",   rooms[i].isAvailable ? "Vacant" : "Occupied");
      fprintf(roomFile, "\n");
    }
    fclose(roomFile);


    printf("Archiving record...\n");
    DELAY(1500);

    FILE *bookingFile = fopen("bookings.txt", "r");
    if (!bookingFile) { printf("Could not open bookings.txt\n"); return; }

    char lines[MAX_BOOKINGS][200];
    int  totalLines = 0;
    while (totalLines < MAX_BOOKINGS &&
            fgets(lines[totalLines], sizeof(lines[totalLines]), bookingFile)) {
        lines[totalLines][strcspn(lines[totalLines], "\n")] = '\0';
        totalLines++;
    }
    fclose(bookingFile);


    int deleteStart = -1;
    int deleteEnd   = -1;
    int inTarget    = 0;

    for (int i = 0; i < totalLines; i++) {
        if (strncmp(lines[i], "------- Guest Info", 18) == 0)
          inTarget = 1;

        if (inTarget && strncmp(lines[i], "Reference No:", 13) == 0) {
          char tmp[10];
          sscanf(lines[i], "Reference No: %9s", tmp);
          if (strcmp(tmp, reservation.referenceNumber) == 0)
              deleteStart = i - 1;
        }

        if (inTarget && strncmp(lines[i], "===========", 11) == 0) {
          if (deleteStart != -1) {
            deleteEnd = i;
            break;
          }
          inTarget = 0;
        }
    }


    bookingFile = fopen("bookings.txt", "w");
    if (!bookingFile) { printf("Could not update bookings.txt\n"); return; }
    for (int i = 0; i < totalLines; i++) {
      if (deleteStart != -1 && i >= deleteStart && i <= deleteEnd)
        continue;
      fprintf(bookingFile, "%s\n", lines[i]);
    }
    fclose(bookingFile);


    printf("\n[Checkout Successful! Room #%03d is now VACANT.]\n",
            reservation.roomNumber);
    printf("========================================\n");
    return;
  }
}

void exitSystem() {
  printf("\n================================================\n");
  printf("    Thank you for visiting ESPLENIN HOTEL!    \n");
  printf("    We hope to see you again soon. Goodbye!   \n");
  printf("================================================\n");
}

// -------------------------------------------------------------------------------------

void printWithCommas(float amount) {
  int wholeNumber = (int)amount;
  int centsPart = (int)((amount - wholeNumber) * 100 + 0.5f);

  if (wholeNumber >= 1000000)
    printf("%d,%03d,%03d.%02d",
           wholeNumber / 1000000,
           (wholeNumber / 1000) % 1000,
           wholeNumber % 1000,
           centsPart);
  else if (wholeNumber >= 1000)
    printf("%d,%03d.%02d",
           wholeNumber / 1000,
           wholeNumber % 1000,
           centsPart);
  else
    printf("%d.%02d", wholeNumber, centsPart);
}

void generateReferenceNumber(char *output) {
  int existingCount = 0;
  FILE *file = fopen("bookings.txt", "r");
  if (file) {
    char line[200];
    while (fgets(line, sizeof(line), file))
      if (strncmp(line, "Reference No:", 13) == 0)
        existingCount++;
    fclose(file);
  }
  sprintf(output, "B%04d", existingCount + 1);
}

int readRoom(FILE *fp, Room *room) {
  char line[150];
  int fieldsFilled = 0;
  memset(room, 0, sizeof(Room));

  while (fgets(line, sizeof(line), fp)) {
    line[strcspn(line, "\n")] = '\0';

    if (strncmp(line, "Room #", 6) == 0) {
      sscanf(line, "Room #%d:", &room->roomNumber);
      fieldsFilled++;
    }
    else if (strncmp(line, "Category:", 9) == 0) {
      sscanf(line, "Category: %29[^\n]", room->category);
      fieldsFilled++;
    }
    else if (strncmp(line, "Bedrooms:", 9) == 0) {
      sscanf(line, "Bedrooms: %d", &room->bedrooms);
      fieldsFilled++;
    }
    else if (strncmp(line, "Price:", 6) == 0) {
      sscanf(line, "Price: %f", &room->pricePerNight);
      fieldsFilled++;
    }
    else if (strncmp(line, "Status:", 7) == 0) {
      room->isAvailable = (strstr(line, "Vacant") != NULL);
      fieldsFilled++;
    }

    if (line[0] == '\0' && fieldsFilled > 0)
      break;
  }

  return fieldsFilled > 0;
}

int readAmenities(const char *filename, Amenity *list, int maxCount) {
  FILE *file = fopen(filename, "r");
  if (!file)
    return 0;

  char line[150];
  int count = 0;
  Amenity *currentItem = &list[count];

  while (fgets(line, sizeof(line), file) && count < maxCount) {
    line[strcspn(line, "\n")] = '\0';

    if (strncmp(line, "Code:", 5) == 0) {
      currentItem = &list[count];
      memset(currentItem, 0, sizeof(Amenity));
      sscanf(line, "Code: %4s", currentItem->code);
    }
    else if (strncmp(line, "Name:", 5) == 0)
      sscanf(line, "Name: %59[^\n]", currentItem->name);
    else if (strncmp(line, "Price:", 6) == 0)
      sscanf(line, "Price: %f", &currentItem->price);
    else if (strncmp(line, "Type:", 5) == 0) {
      sscanf(line, "Type: %19[^\n]", currentItem->type);
      count++;
    }
  }

  fclose(file);
  return count;
}

int parseAmenityCodes(const char *line, char codes[][5], int maxCodes) {
  char lineCopy[200];
  int count = 0;

  memset(codes, 0, sizeof(char) * maxCodes * 5);
  strncpy(lineCopy, line, sizeof(lineCopy) - 1);
  lineCopy[sizeof(lineCopy) - 1] = '\0';

  char *codeText = strchr(lineCopy, ':');
  if (!codeText)
    return 0;

  codeText++;
  char *tok = strtok(codeText, ",");
  while (tok && count < maxCodes) {
    while (isspace((unsigned char)*tok))
      tok++;

    char *end = tok + strlen(tok);
    while (end > tok && isspace((unsigned char)*(end - 1))) {
      end--;
      *end = '\0';
    }

    if (strlen(tok) > 0 && strlen(tok) < 5) {
      strncpy(codes[count], tok, 4);
      codes[count][4] = '\0';
      count++;
    }

    tok = strtok(NULL, ",");
  }

  return count;
}

int findAmenityByCode(const char *code, Amenity *foundAmenity) {
  const char *amenFiles[3] = {
    "Amenities/convenienceAmenite.txt",
    "Amenities/poolAmenite.txt",
    "Amenities/spaAmenite.txt"
  };

  for (int f = 0; f < 3; f++) {
    Amenity list[10];
    int count = readAmenities(amenFiles[f], list, 10);
    for (int a = 0; a < count; a++) {
      if (strcasecmp(list[a].code, code) == 0) {
        *foundAmenity = list[a];
        return 1;
      }
    }
  }

  return 0;
}

float calculateAmenitiesTotal(char codes[][5], int codeCount, int numberOfDays, int numberOfGuests) {
  float total = 0.0f;
  int chargeableNights = numberOfDays > 0 ? numberOfDays - 1 : 0;

  for (int c = 0; c < codeCount; c++) {
    Amenity amenity;
    if (!findAmenityByCode(codes[c], &amenity))
      continue;

    if (strcmp(amenity.type, "PerNight") == 0)
      total += amenity.price * chargeableNights;
    else
      total += amenity.price * numberOfGuests;
  }

  return total;
}

int findBooking(const char *referenceNumber, Reservation *reservation) {
  FILE *file = fopen("bookings.txt", "r");
  if (!file) {
    printf("Could not open bookings.txt\n");
    return 0;
  }

  char line[200];
  int insideBlock = 0;
  int found = 0;

  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = '\0';

    if (strncmp(line, "------- Guest Info", 18) == 0) {
      insideBlock = 1;
      memset(reservation, 0, sizeof(Reservation));
    }

    if (!insideBlock)
      continue;

    if (strncmp(line, "Reference No:", 13) == 0)
      sscanf(line, "Reference No: %9s", reservation->referenceNumber);
    else if (strncmp(line, "Room Type:", 10) == 0)
      sscanf(line, "Room Type: %29[^\n]", reservation->roomType);
    else if (strncmp(line, "Main Guest:", 11) == 0)
      sscanf(line, "Main Guest: %49[^\n]", reservation->guestName);
    else if (strncmp(line, "No of Guest:", 12) == 0)
      sscanf(line, "No of Guest: %d", &reservation->numberOfGuests);
    else if (strncmp(line, "Check-In:", 9) == 0)
      sscanf(line, "Check-In: %19[^\n]", reservation->checkIn);
    else if (strncmp(line, "Checkout:", 9) == 0)
      sscanf(line, "Checkout: %19[^\n]", reservation->checkOut);
    else if (strncmp(line, "No of Days:", 11) == 0)
      sscanf(line, "No of Days: %d", &reservation->numberOfDays);
    else if (strncmp(line, "Room #:", 7) == 0)
      sscanf(line, "Room #: %d", &reservation->roomNumber);
    else if (strncmp(line, "Room Rate:", 10) == 0)
      sscanf(line, "Room Rate: %f", &reservation->roomRate);
    else if (strncmp(line, "Amenities Total:", 16) == 0)
      sscanf(line, "Amenities Total: %f", &reservation->amenitiesTotal);
    else if (strncmp(line, "Final Amount:", 13) == 0)
      sscanf(line, "Final Amount: %f", &reservation->finalAmount);
    else if (strncmp(line, "Status:", 7) == 0)
      reservation->isPaid = (strstr(line, "Not Paid") == NULL &&
                             strstr(line, "Paid") != NULL);

    if (strncmp(line, "===========", 11) == 0 && insideBlock) {
      insideBlock = 0;
      if (strcmp(reservation->referenceNumber, referenceNumber) == 0) {
        found = 1;
        break;
      }
    }
  }
  return found;
}

void updateBookingPayment(Reservation *reservation) {
  FILE *file = fopen("bookings.txt", "r");
  if (!file) { printf("Could not open bookings.txt\n"); return; }

  char lines[MAX_BOOKINGS][200];
  int totalLines = 0;
  while (totalLines < MAX_BOOKINGS &&
         fgets(lines[totalLines], sizeof(lines[totalLines]), file)) {
    lines[totalLines][strcspn(lines[totalLines], "\n")] = '\0';
    totalLines++;
  }
  fclose(file);

  int inTargetBlock = 0;
  for (int i = 0; i < totalLines; i++) {
    if (strncmp(lines[i], "Reference No:", 13) == 0) {
      char tmp[10];
      sscanf(lines[i], "Reference No: %9s", tmp);
      inTargetBlock = (strcmp(tmp, reservation->referenceNumber) == 0);
    }

    if (!inTargetBlock)
      continue;

    if (strncmp(lines[i], "Amenities Total:", 16) == 0)
      sprintf(lines[i], "Amenities Total: %.2f", reservation->amenitiesTotal);
    else if (strncmp(lines[i], "Final Amount:", 13) == 0)
      sprintf(lines[i], "Final Amount: %.2f", reservation->finalAmount);
    else if (strncmp(lines[i], "Status:", 7) == 0)
      sprintf(lines[i], "Status: %s", reservation->isPaid ? "Paid" : "Not Paid");
    else if (strncmp(lines[i], "Method:", 7) == 0)
      sprintf(lines[i], "Method: %s", reservation->paymentMethod);
    else if (strncmp(lines[i], "Amount Received:", 16) == 0)
      sprintf(lines[i], "Amount Received: %.2f", reservation->amountReceived);
    else if (strncmp(lines[i], "Change:", 7) == 0)
      sprintf(lines[i], "Change: %.2f", reservation->change);

    if (strncmp(lines[i], "===========", 11) == 0)
      inTargetBlock = 0;
  }

  file = fopen("bookings.txt", "w");
  if (!file) {
    printf("Could not save payment.\n");
    return;
  }

  for (int i = 0; i < totalLines; i++)
    fprintf(file, "%s\n", lines[i]);
  fclose(file);
}

char *formatPrice(float amount, char *buffer) {
    int wholeNumber = (int)amount;
    int centsPart   = (int)((amount - wholeNumber) * 100 + 0.5f);

    if (wholeNumber >= 1000000)
      sprintf(buffer, "%d,%03d,%03d.%02d",
        wholeNumber / 1000000,
        (wholeNumber / 1000) % 1000,
        wholeNumber % 1000,
        centsPart);
    else if (wholeNumber >= 1000)
      sprintf(buffer, "%d,%03d.%02d",
        wholeNumber / 1000,
        wholeNumber % 1000,
        centsPart);
    else
        sprintf(buffer, "%d.%02d", wholeNumber, centsPart);

    return buffer;
}

void receiptGenerator(Reservation *reservation, int methodPick) {
  int receiptCount = 0;



  char testPath[50];
  for (int i = 1; i <= 9999; i++) {
    sprintf(testPath, "Receipts/RCPT-%04d.txt", i);
    FILE *test = fopen(testPath, "r");
    if (test) { fclose(test); receiptCount = i; }
    else break;
  }

  int    orNumber = receiptCount + 1;
  char   orString[12];
  sprintf(orString, "RCPT-%04d", orNumber);

  char filename[50];
  sprintf(filename, "Receipts/%s.txt", orString);

  FILE *file = fopen(filename, "w");
  if (!file) {
    printf("Could not save receipt. Make sure the 'Receipts' folder exists.\n");
    return;
  }

  char priceBuffer[20];

  fprintf(file, "================================================\n");
  fprintf(file, "           ESPLENIN HOTEL RECEIPT\n");
  fprintf(file, "================================================\n");
  fprintf(file, "OR No.        : %s\n",  orString);
  fprintf(file, "Reference No  : %s\n",  reservation->referenceNumber);
  fprintf(file, "Guest Name    : %s\n",  reservation->guestName);
  fprintf(file, "Room #        : %03d\n", reservation->roomNumber);
  fprintf(file, "Room Type     : %s\n",  reservation->roomType);
  fprintf(file, "------------------------------------------------\n");
  fprintf(file, "Room Rate     : PHP %s\n", formatPrice(reservation->roomRate,       priceBuffer));
  fprintf(file, "Amenities     : PHP %s\n", formatPrice(reservation->amenitiesTotal, priceBuffer));
  fprintf(file, "------------------------------------------------\n");
  fprintf(file, "TOTAL DUE     : PHP %s\n", formatPrice(reservation->finalAmount,    priceBuffer));
  fprintf(file, "Amount Paid   : PHP %s\n", formatPrice(reservation->amountReceived, priceBuffer));
  fprintf(file, "Change        : PHP %s\n", formatPrice(reservation->change,         priceBuffer));
  fprintf(file, "Method        : %s\n",  reservation->paymentMethod);
  fprintf(file, "================================================\n");
  fprintf(file, "       Thank you for choosing us!\n");
  fprintf(file, "================================================\n");

  fclose(file);

  printf("OR No.: %s\n", orString);
  printf("Receipt saved as %s\n", filename);
  printf("==========================================\n");
}

char inqPickCategory(const char *prompt){
  char pick;
  printf("%s\n", prompt);
  printf("  [A] - Classic\n");
  printf("  [B] - De Luxe\n");
  printf("  [C] - Suite\n");
  printf("  [D] - Imperial Grand\n");
  printf("  [0] - Back\n");
  printf("Choice: ");
  scanf("%c", &pick);
  while (getchar() != '\n');
  return toupper(pick);
}

int readAllRooms(Room *rooms) {
  FILE *file = fopen("rooms.txt", "r");
  if (!file) {
    printf("Could not open rooms.txt\n");
    return 0;
  }

  char line[150];
  int count = 0;
  int filled = 0;
  Room *r = &rooms[count];

  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = '\0';
    line[strcspn(line, "\r")] = '\0';

    if (strncmp(line, "Room #", 6) == 0) {
      r = &rooms[count];
      memset(r, 0, sizeof(Room));
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

  if (filled)
    count++;

  fclose(file);
  return count;
}
