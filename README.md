# **Eklamon**
Wearable Blood Pressure Monitor using LoLin32 Lite + MAX30102 with Cloud ML Processing.

## **Project Overview**
![Alt Text](https://raw.githubusercontent.com/stnrliza/eklamon/main/images/eklamon.jpeg)

Eklamon (a wrist-worn blood pressure prediction device) uses the **MAX30102 sensor** and a **Random Forest model** to process IR and RED readings as a **cuffless blood pressure measurement innovation**, leveraging **WiFi connectivity** for data storage and machine learning computation via the campus remote SSH server.


## **Key Features**
- Cuffless Blood Pressure (BP) measurement.
- Wireless data transmission using WiFi - HTTP POST to remote ML server.
- Real-time prediction using Random Forest with 90% accuracy.
- Low-power wearable, lasts up to 3 hours constant usage.


## **System Architecture Diagram**
![Alt Text](https://raw.githubusercontent.com/stnrliza/eklamon/main/images/architecture-diagram.png)

## **Hardware Components**
| Component | Description |
|----------|-------------|
| **Microcontroller: Wemos LoLin32 Lite** | WiFi-enabled microcontroller with an integrated Battery Management System (BMS). |
| **Sensor: MAX30102** | Photoplethysmography (PPG) sensor that reads IR and RED signals, processed by an ML model to predict blood pressure. |
| **Display: OLED 0.96 inch** | Compact, low-power OLED screen used to display blood pressure predictions. |
| **Battery: LiPo 3.7V 400mAh (401225)** | Small LiPo battery powering all components. |
| **Switch: SPDT Slide Switch** | Serves as the device’s on/off power control. |
| **Button: Tactile Push Button Switch** | Used to trigger the start of the measurement. |


## **Wiring Diagram**
![Alt Text](https://raw.githubusercontent.com/stnrliza/eklamon/main/images/wiring.png)

## **3D Design**
![Alt Text](https://raw.githubusercontent.com/stnrliza/eklamon/main/images/3D.gif)

## **User Flow**
![Alt Text](https://raw.githubusercontent.com/stnrliza/eklamon/main/images/user-flow.png)


## **ML Model Summary**
The machine learning pipeline for blood pressure prediction is built using a RandomForestRegressor model with Optuna-based hyperparameter tuning.  
The workflow includes data collection, preprocessing, feature extraction, model training, evaluation, and deployment to a Flask server.

### **Data Processing Pipeline**
1. **PPG Data Collection**  
   Raw IR and RED signals are collected from the MAX30102 sensor.

2. **Signal Preprocessing**  
   - DC removal  
   - Bandpass filtering  
   - Normalization  

3. **Feature Extraction & Labeling**  
   Statistical and morphological features are extracted from the filtered PPG signals and paired with SBP/DBP labels.

### **Model Training**
- Algorithm: **RandomForestRegressor**  
- Hyperparameter tuning: **Optuna**  
- Targets predicted: **SBP**, **DBP**, and **Pulse**

### **Model Evaluation**
The model is evaluated using metrics such as:  
- **MAE (Mean Absolute Error)**  
- **R² Score**  
- Additional validation checks

The best-performing model is exported as a **`.pkl` file** for deployment.

### **Deployment**
The final model (`model.pkl`) is deployed on a Flask server, accessed via an API endpoint.  
The microcontroller sends PPG features to this endpoint and receives predicted BP values in return.



## **Collaborators & Roles**
- Siti Nurhaliza (me) — Team lead, electrical engineer, ML engineer
- Nabila Fairuz Romadhona — ML lead, model deployment engineer
- Angeline Annabelle Kacaribu — Clinical data acquisition, 3D designer
