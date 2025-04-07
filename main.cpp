#include <stdio.h>
#include <cstdlib>
#include <math.h>

const double baseResistors[] = {1, 1.2, 1.5, 1.8, 2.2, 2.7, 3.3, 3.9, 4.7, 5.6, 6.8, 8.1};

typedef struct resistorCombo {
  resistorCombo* R1;
  resistorCombo* R2;
  double value;
  int type;       // 0 = solo, 1 = series, 2 = parallel
} resistorCombo;

typedef struct {
  resistorCombo* resistances;
  int numStored, maxStorable;
} resistorList;

void appendResistor(resistorList* resistors, double newValue, resistorCombo* newR1, resistorCombo* newR2, int newType) {
  if (resistors->maxStorable <= resistors->numStored) {
    int newSize = 3*resistors->maxStorable/2;
    if (newSize == resistors->maxStorable) newSize += 3;
  
    resistorCombo* newResistorArray = (resistorCombo*)realloc(resistors->resistances, sizeof(resistorCombo)*newSize);

    if (!newResistorArray) {
      printf("Couldn't allocate memory for resistor array during resize!\n");
      return;
    }

    resistors->resistances = newResistorArray;
    resistors->maxStorable = newSize;
  }

  resistorCombo newResistor;
  newResistor.R1 = newR1;
  newResistor.R2 = newR2;
  newResistor.value = newValue;
  newResistor.type = newType;

  resistors->resistances[resistors->numStored] = newResistor;
  resistors->numStored += 1;
}

void initResistorList(resistorList* resistors) {
  if (resistors->resistances != 0) {
    resistors->maxStorable = 0;
    resistors->numStored = 0;
    free(resistors->resistances);
  }

  resistorCombo* resistorArray = (resistorCombo*)malloc(sizeof(resistorCombo)*12*5);

  if (!resistorArray) {
    printf("Couldn't allocate memory for resistor array!\n");
    return;
  }

  resistors->resistances = resistorArray;
  resistors->numStored = 0;
  resistors->maxStorable = 60;

  for (int i=0; i<=6; i++) {
    for (int j=0; j<12; j++) {
      appendResistor(resistors, baseResistors[j]*pow(10.0, i), 0, 0, 0); 
    }
  }
}

void printResistorCombo(resistorCombo* combo) {
  if (combo->R2 == combo || combo->R1 == combo) {
    printf("Self reference!\n");
    return;
  }

  if (combo->type == 0)
    printf("%.2lf", combo->value);
  else {
    printf("(");
    printResistorCombo(combo->R1); 
    if (combo->type == 1) printf("+");
    if (combo->type == 2) printf("||");
    printResistorCombo(combo->R2); 
    printf(")");
  }
}

void destroyResistorList(resistorList* resistors) {
  free(resistors->resistances);
}

int main() {
  resistorList resistors;
  resistors.resistances = 0;
  initResistorList(&resistors);

  resistorList resistors2;
  resistorCombo* resistorArray = (resistorCombo*)malloc(sizeof(resistorCombo)*5);
  if (!resistorArray) {
    printf("Couldn't allocate array for second resistor list!\n");
    return 1;
  }
  resistors2.resistances = resistorArray;
  resistors2.numStored = 0; 
  resistors2.maxStorable = 5;

  for (int i=0; i<resistors.numStored; i++) {
    for (int j=0; j<=i; j++) {
      resistorCombo* R1 = &resistors.resistances[i];
      resistorCombo* R2 = &resistors.resistances[j];
      double R1val = (*R1).value;
      double R2val = (*R2).value;
      double newSeries = R1val + R2val;
      double newParallel = 1/(1/R1val + 1/R2val);

      // if R1 and R2 are over 2 orders of magnitude apart, don't bother
      // cause we're just making micro adjustments
      if ( (R1val*100 > R2val) && (R1val < R2val*100) ) {
        appendResistor(&resistors2, newSeries, R1, R2, 1);
        appendResistor(&resistors2, newParallel, R1, R2, 2);
      }
    }
  }

  double goal;
  printf("Enter the desired resistor (ohm): ");
  scanf("%lf", &goal);
  for (int i=0; i<resistors2.numStored; i++) {
    for (int j=0; j<i; j++) {
      double d1 = goal - resistors2.resistances[i].value;
      double d2 = goal - resistors2.resistances[j].value;

      if (d1 < 0) d1 = -d1;
      if (d2 < 0) d2 = -d2;
      if (d1 < d2) {
        resistorCombo temp = resistors2.resistances[i];
        resistors2.resistances[i] = resistors2.resistances[j];
        resistors2.resistances[j] = temp; 
      }
    }
  }

  for (int i=0; i<10; i++) {
    resistorCombo curCombo = resistors2.resistances[i];
    printf("%d: %.2lf is %.2lf\n", i+1, goal, curCombo.value);
    printf("  Construction: ");
    printResistorCombo(&curCombo);
    printf("\n");
  }

  destroyResistorList(&resistors);
  destroyResistorList(&resistors2);

  return 0;
}
