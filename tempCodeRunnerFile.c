      int valid = 0;

      while(!valid) {
        printf("\nWould you like to cancel a reservation? [y/n]: ");
        scanf(" %c", &cancelChoice);
        while (getchar() != '\n');

        if (toupper(cancelChoice) == 'Y') break;
        if (toupper(cancelChoice) == 'N') return;

        printf("Invalid choice. Please enter Y or N only.\n");
      }

      if (tolower(cancelChoice) == 'y') {
        char targetRef[10];
        printf("Enter reference number to cancel: ");
        scanf("%9s", targetRef);
        while (getchar() != '\n');

        Reservation res;
        if (!findBooking(targetRef, &res)) {
          printf("[ERROR] Reference number %s not found.\n", targetRef);
          return;
        }