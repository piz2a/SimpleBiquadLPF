import React, { useRef, useState, useEffect, useCallback } from 'react';

// JUCE 통신용 스로틀 유틸리티
function useThrottle(callback: (...args: any[]) => void, delay: number) {
  const lastCall = useRef<number>(0);
  return useCallback((...args: any[]) => {
    const now = Date.now();
    if (now - lastCall.current >= delay) {
      lastCall.current = now;
      callback(...args);
    }
  }, [callback, delay]);
}

// 로그 스케일 변환 수식: $f = f_{min} \cdot (f_{max}/f_{min})^p$
const linearToLog = (p: number, min: number, max: number) => 
  min * Math.pow(max / min, p);

const logToLinear = (f: number, min: number, max: number) => 
  Math.log(f / min) / Math.log(max / min);

interface KnobProps {
  label: string;
  min: number;
  max: number;
  initialValue: number;
  unit: string;
  isLog?: boolean;
  onUpdate: (val: number) => void;
  decimalPlaces?: number;
}

const Knob = ({ label, min, max, initialValue, unit, isLog, onUpdate, decimalPlaces = 0 }: KnobProps) => {
  const [value, setValue] = useState(initialValue);
  const [isEditing, setIsEditing] = useState(false);
  const knobRef = useRef<HTMLDivElement>(null);
  const startY = useRef(0);
  const startVal = useRef(0);

  const updateVal = useCallback((newVal: number) => {
    const clamped = Math.max(min, Math.min(max, newVal));
    const fixed = parseFloat(clamped.toFixed(decimalPlaces));
    setValue(fixed);
    onUpdate(fixed);
  }, [min, max, decimalPlaces, onUpdate]);

  const onMouseDown = (e: React.MouseEvent) => {
    if (isEditing) return;
    startY.current = e.clientY;
    // 로그 스케일일 경우 선형적인 0-1 비율로 변환하여 시작점 저장
    startVal.current = isLog ? logToLinear(value, min, max) : (value - min) / (max - min);

    const onMouseMove = (moveEvent: MouseEvent) => {
      const deltaY = startY.current - moveEvent.clientY;
      const sensitivity = 200; // 200px 드래그 시 전체 범위 이동
      const deltaPercent = deltaY / sensitivity;
      const newPercent = Math.max(0, Math.min(1, startVal.current + deltaPercent));

      if (isLog) {
        updateVal(linearToLog(newPercent, min, max));
      } else {
        updateVal(min + newPercent * (max - min));
      }
    };

    const onMouseUp = () => {
      window.removeEventListener('mousemove', onMouseMove);
      window.removeEventListener('mouseup', onMouseUp);
    };

    window.addEventListener('mousemove', onMouseMove);
    window.addEventListener('mouseup', onMouseUp);
  };

  const handleManualInput = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      const val = parseFloat((e.target as HTMLInputElement).value);
      if (!isNaN(val)) updateVal(val);
      setIsEditing(false);
    }
  };

  // 노브 회전 각도 계산 (-135도 ~ 135도)
  const percent = isLog ? logToLinear(value, min, max) : (value - min) / (max - min);
  const rotation = percent * 270 - 135;

  return (
    <div className="flex flex-col items-center gap-3">
      <span className="text-[10px] font-black tracking-widest text-cyan-500 uppercase">{label}</span>
      
      <div 
        ref={knobRef}
        onMouseDown={onMouseDown}
        className="relative w-28 h-28 rounded-full bg-slate-900 shadow-[5px_5px_15px_#050505,-5px_-5px_15px_#1a1a1a] flex items-center justify-center cursor-ns-resize group"
      >
        {/* Progress Ring (SVG) */}
        <svg className="absolute w-full h-full -rotate-90" viewBox="0 0 100 100">
          <circle cx="50" cy="50" r="45" fill="none" stroke="#1e293b" strokeWidth="4" />
          <circle 
            cx="50" cy="50" r="45" fill="none" stroke="#06b6d4" strokeWidth="4"
            strokeDasharray={282.7}
            strokeDashoffset={282.7 - (282.7 * percent)}
            strokeLinecap="round"
            className="transition-none drop-shadow-[0_0_5px_#06b6d4]"
          />
        </svg>

        {/* Knob Face */}
        <div className="w-20 h-20 rounded-full bg-gradient-to-br from-slate-800 to-slate-950 shadow-lg flex flex-col items-center justify-center relative">
          {/* Indicator Dot */}
          <div 
            className="absolute inset-0 transition-none"
            style={{ transform: `rotate(${rotation}deg)` }}
          >
            <div className="absolute top-2 left-1/2 -translate-x-1/2 w-1.5 h-1.5 bg-cyan-400 rounded-full shadow-[0_0_8px_#22d3ee]" />
          </div>

          {/* Value Text */}
          {isEditing ? (
            <input 
              autoFocus
              className="w-16 bg-transparent text-center text-white font-bold outline-none border-b border-cyan-500"
              defaultValue={value}
              onKeyDown={handleManualInput}
              onBlur={() => setIsEditing(false)}
            />
          ) : (
            <span 
              onDoubleClick={() => setIsEditing(true)}
              className="text-lg font-black text-slate-100 cursor-text tracking-tighter"
            >
              {value}
            </span>
          )}
          <span className="text-[8px] font-bold text-slate-500">{unit}</span>
        </div>
      </div>
    </div>
  );
};

export default function SimpleBiquadLPF() {
  const sendParamToJuce = useThrottle((paramId: string, value: number) => {
    if (window.__juce_backend) {
      window.__juce_backend.callNativeFunction("updateParameter", [paramId, value]);
    } else {
      console.log(`[JUCE] ${paramId}: ${value}`);
    }
  }, 16);

  return (
    <div className="w-[480px] h-[320px] bg-black bg-[radial-gradient(circle_at_center,_#111_0%,_#000_100%)] flex flex-col items-center justify-between p-8 overflow-hidden font-sans border border-slate-800">
      
      {/* Cyberpunk Header */}
      <div className="w-full flex justify-between items-center border-b border-cyan-900/30 pb-4">
        <h1 className="text-xl font-black italic tracking-tighter text-transparent bg-clip-text bg-gradient-to-r from-cyan-400 to-blue-600 drop-shadow-[0_0_10px_rgba(34,211,238,0.4)]">
          SIMPLE_BIQUAD_LPF
        </h1>
        <div className="flex gap-1">
          <div className="w-2 h-2 bg-cyan-500 animate-pulse" />
          <div className="w-8 h-2 bg-slate-800" />
        </div>
      </div>

      {/* Control Section */}
      <div className="flex gap-16 items-center flex-1">
        <Knob 
          label="Cutoff" 
          min={20} 
          max={22050} 
          initialValue={1000} 
          unit="Hz" 
          isLog={true}
          onUpdate={(v) => sendParamToJuce("freqHz", v)} 
        />
        <Knob 
          label="Resonance" 
          min={0.0} 
          max={12.0} 
          initialValue={0.7} 
          unit="dB" 
          decimalPlaces={1}
          onUpdate={(v) => sendParamToJuce("resonance", v)} 
        />
      </div>

      {/* Footer Decoration */}
      <div className="w-full flex justify-between text-[8px] font-mono text-slate-600 tracking-[0.3em] uppercase">
        <span>S6_Selective_Architecture_v.2026</span>
        <span>Internal_Processing_64bit</span>
      </div>
    </div>
  );
}