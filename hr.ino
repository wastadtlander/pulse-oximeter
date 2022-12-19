#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(9600);
  displaySetup(0, 0, 0, {});
}

void loop() {
  int hrData[300];
  int irData[300];
  for (int i = 0; i < 300; i++) {
    hrData[i] = analogRead(0);
    irData[i] = analogRead(1);
//    display.drawPixel(i, hrData[i]/15, WHITE);
//    display.display();
    delay(20);
  }
  int HR = calculateHR(hrData);
  int SpO = calculateSpO2(hrData, irData);
  float pulseIntensity = calculatePI(hrData, irData);
  displaySetup(HR, SpO, pulseIntensity, hrData);
}

int calculateHR(int hrData[]) {
    int peakDiff[20] = {};
    int peakDiffCounter = 0;
    int peakDiffIndex = 0;
    for (int i = 3; i < 297; i++) {
      if (hrData[i - 1] <= hrData[i] && hrData[i + 1] <= hrData[i] && hrData[i - 2] <= hrData[i] && hrData[i + 2] <= hrData[i] && hrData[i - 3] <= hrData[i] && hrData[i + 3] <= hrData[i] && hrData[i] != 0 && hrData[i] > 300) {
        if (peakDiffCounter > 20 && peakDiffCounter < 60) {
          peakDiff[peakDiffIndex] += peakDiffCounter;
          peakDiffIndex++;
        }
        peakDiffCounter = 0;
      } else {
        peakDiffCounter++;
      }
    }
 
    int peakDiffSum = 0;
    int peakDiffMin = peakDiff[0];
   
    for (int i = 1; i < peakDiffIndex; i++) {
      if (peakDiff[i] < peakDiffMin)
        peakDiffMin = peakDiff[i];
    }
    for (int i = 0; i < peakDiffIndex; i++) {
      if (peakDiff[i] != peakDiffMin)
        peakDiffSum += peakDiff[i];
    }
    peakDiffSum /= (peakDiffIndex - 1);
    peakDiffSum = 60*(1/(peakDiffSum/50.0));
    Serial.print("HR: ");
    Serial.println(peakDiffSum);
    return peakDiffSum;
}

int calculateSpO2(int hrData[], int irData[]) {
  float minHR = hrData[0];
  float minIR = irData[0];
  float maxHR = hrData[0];
  float maxIR = hrData[0];
  double avgHR = 0;
  double avgIR = 0;              

  for (int i = 1; i < 300; i++) {
    if (hrData[i] < minHR)
      minHR = hrData[i];
    if (irData[i] < minIR)
      minIR = irData[i];
    if (hrData[i] > maxHR)
      maxHR = hrData[i];
    if (irData[i] > maxIR)
      maxIR = irData[i];  
  avgHR += hrData[i];
  avgIR += irData[i];
  }

  float ppHR = maxHR - minHR;
  float ppIR = maxIR - minIR;
  
  avgHR /= 300.0;
  avgIR /= 300.0;
 
  float R = (ppHR/avgHR)/(ppIR/avgIR);

  int SpO = ((110) + (-12*R));
  Serial.print("SpO2: ");
  Serial.println(SpO);

  if (SpO < 93)
    analogWrite(5, 100);
  else
    analogWrite(5, 0);
  
  return SpO;
}

double calculatePI(int hrData[], int irData[]) {
  float minHR = hrData[0];
  float maxHR = irData[0];
  double avgHR = 0;          

  for (int i = 1; i < 300; i++) {
    if (hrData[i] < minHR)
      minHR = hrData[i];
    if (hrData[i] > maxHR)
      maxHR = hrData[i];
    avgHR += hrData[i];
  }
  float ppHR = maxHR - minHR;
 
  avgHR /= 300.0;
  avgHR -= 82;

  float pulseIntensity = ((ppHR/avgHR)*100.0)/10.0;
  Serial.print("PI: ");
  Serial.println(pulseIntensity);
  return pulseIntensity;
}

void displaySetup(int HR, int SpO, float pulseIntensity, int hrData[]) {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
 
  display.setCursor(0, 25);
  display.print("Heart Rate: ");
  display.print(HR);
  display.print(" BPM");

  display.setCursor(0, 40);
  display.print("SpO2: ");
  display.print(SpO);
  display.print("%");
 
  display.setCursor(0, 55);
  display.print("PI: ");
  display.print(pulseIntensity);
  display.print("%");

//  for (int i = 0; i < 128; i++) {
//    display.drawPixel(i, hrData[i]/15, WHITE);
//  }

  display.display();
}
