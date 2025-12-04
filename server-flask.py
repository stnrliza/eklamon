# ================================================
# server-flask.py
# Flask Server + Preprocessing (Filtering & Normalisation)
# + Feature Extraction + RF Prediction
# ================================================

from flask import Flask, request, jsonify
import os
import datetime
import logging
import pandas as pd
import numpy as np
from scipy.signal import butter, filtfilt, find_peaks
from sklearn.preprocessing import MinMaxScaler
import joblib
from flask_cors import CORS

# ===== Inisialisasi Flask =====
app = Flask(__name__)
CORS(app)
logging.basicConfig(level=logging.INFO)

# ===== Path utama =====
BASE_DIR = os.path.join(os.getcwd(), "UJI_KLINIS")
os.makedirs(BASE_DIR, exist_ok=True)

# ===== Load Model =====
MODEL_PATHS = {
    "DBP": "RF_model_DBP.pkl",
    "SBP": "RF_model_SBP.pkl",
    "Pulse": "RF_model_Pulse.pkl"
}

MODELS = {}
for key, path in MODEL_PATHS.items():
    if os.path.exists(path):
        MODELS[key] = joblib.load(path)
        print(f"[OK] Model {key} loaded from {path}")
    else:
        print(f"[WARN] Model {path} not found, skipping.")

# ===== Utility Functions =====
def remove_dc(signal):
    return signal - np.mean(signal)

def bandpass_filter(signal, fs=25, low=0.5, high=5.0, order=4):
    nyq = 0.5 * fs
    b, a = butter(order, [low/nyq, high/nyq], btype='band')
    return filtfilt(b, a, signal)

def normalize_signal(signal):
    scaler = MinMaxScaler()
    return scaler.fit_transform(signal.reshape(-1, 1)).flatten()

def extract_ppg_features(signal, time, fs=25):
    features = {}
    features["mean"] = np.mean(signal)
    features["median"] = np.median(signal)
    features["std"] = np.std(signal)
    features["var"] = np.var(signal)
    features["max"] = np.max(signal)
    features["min"] = np.min(signal)
    features["p2p_amplitude"] = np.max(signal) - np.min(signal)

    peaks, _ = find_peaks(signal, distance=0.3 * fs)
    features["num_beats"] = len(peaks)

    if len(peaks) > 1:
        rr_intervals = np.diff(time[peaks])
        features["mean_rr"] = np.mean(rr_intervals)
        features["std_rr"] = np.std(rr_intervals)
        features["heart_rate_bpm"] = 60 / np.mean(rr_intervals)
    else:
        features["mean_rr"] = 0
        features["std_rr"] = 0
        features["heart_rate_bpm"] = 0

    return features

# ===== Route upload =====
@app.route("/upload", methods=["POST"])
def upload_csv():
    try:
        print("[INFO] Request diterima dari:", request.remote_addr)
        raw_data = request.data.decode("utf-8")
        if not raw_data.strip():
            return jsonify({"status": "error", "message": "No data received"}), 400

        # Simpan data mentah
        filename = datetime.datetime.now().strftime("%Y%m%d-%H%M%S") + ".csv"
        filepath = os.path.join(BASE_DIR, filename)
        with open(filepath, "w", encoding="utf-8") as f:
            f.write(raw_data)
        print(f"[OK] Data tersimpan di {filepath}")

        # Baca CSV dengan aman (UTF-8 BOM-safe)
        df = pd.read_csv(filepath, encoding="utf-8-sig")
        if "IR" not in df.columns:
            return jsonify({"status": "error", "message": "No IR column found"}), 400

        ir = df["IR"].values

        # Guard: sinyal IR flat → gagal
        if np.all(ir == ir[0]):
            return jsonify({"status": "error", "message": "IR signal flat"}), 400

        # ======== Proses Data ========
        fs = 25  # FIX: harus sama dengan ESP32

        ir_dc = remove_dc(ir)
        ir_filtered = bandpass_filter(ir_dc, fs)
        ir_norm = normalize_signal(ir_filtered)

        df["Filtered_IR"] = ir_filtered
        df["Normalized_IR"] = ir_norm
        filtered_path = filepath.replace(".csv", "_normalized.csv")
        df.to_csv(filtered_path, index=False)

        # Ekstraksi fitur
        time = np.arange(len(df)) / fs
        features = extract_ppg_features(ir_norm, time, fs)
        feature_df = pd.DataFrame([features])

        # ======== Prediksi ========
        predictions = {}
        for key, model in MODELS.items():
            try:
                model_features = model.feature_names_in_ if hasattr(model, "feature_names_in_") else feature_df.columns
                X_input = feature_df[model_features]
                predictions[key] = float(model.predict(X_input)[0])
            except Exception as e:
                predictions[key] = None
                print(f"[WARN] Prediksi {key} gagal:", e)

        # Simpan hasil prediksi
        pred_file = filepath.replace(".csv", "_prediksi.csv")
        feature_df["SBP_pred"] = predictions.get("SBP")
        feature_df["DBP_pred"] = predictions.get("DBP")
        feature_df["Pulse_pred"] = predictions.get("Pulse")
        feature_df.to_csv(pred_file, index=False)
        print(f"[OK] Prediksi disimpan di {pred_file}")

        return jsonify({
            "status": "success",
            "file": filename,
            "predictions": predictions
        }), 200

    except Exception as e:
        print("[ERROR]", e)
        return jsonify({"status": "error", "message": str(e)}), 500


# ===== Main =====
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5001)
