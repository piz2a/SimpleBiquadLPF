import React, { useRef, useState, useEffect } from 'react';
import { Button } from '@/components/ui/button';
import * as Juce from './juce';

// 로그 스케일 변환 수식
const linearToLog = (p: number, min: number, max: number) => 
  min * Math.pow(max / min, p);

const logToLinear = (f: number, min: number, max: number) => 
  Math.log(f / min) / Math.log(max / min);

interface KnobProps {
  label: string;
  paramId: string; // parameter ID in JUCE APVTS
  min: number;
  max: number;
  initialValue: number;
  unit: string;
  isLog?: boolean;
  decimalPlaces?: number;
}

const Knob = ({ label, paramId, min, max, initialValue, unit, isLog, decimalPlaces = 0 }: KnobProps) => {
  const [value, setValue] = useState(initialValue);
  const [isEditing, setIsEditing] = useState(false);
  const knobRef = useRef<HTMLDivElement>(null);

  const sliderStateRef = useRef<Juce.SliderState>(null);
  const isDragging = useRef(false);

  // Load SliderState from JUCE and set up listener
  useEffect(() => {
    const state = Juce.getSliderState(paramId);
    console.log(`✅ SliderState for ${paramId} obtained:`, state); 
    sliderStateRef.current = state;

    const listener = () => {
      if (!isDragging.current) {
        const scaledValue = state.getScaledValue();
        console.log(`🔔 ${paramId} scaled value from JUCE:`, scaledValue);
        setValue(parseFloat(scaledValue.toFixed(decimalPlaces)));
      }
    };

    const listenerId = state.valueChangedEvent.addListener(listener);
    listener(); // 초기 상태 동기화

    // Cleanup
    return () => state.valueChangedEvent.removeListener(listenerId);
  }, [paramId, min, max, isLog, decimalPlaces]);

  // Drag handling
  const onMouseDown = (e: React.MouseEvent) => {
    if (isEditing || !sliderStateRef.current) return;
    
    isDragging.current = true;
    const startY = e.clientY;
    const startNorm = sliderStateRef.current.getNormalisedValue();

    const onMouseMove = (moveEvent: MouseEvent) => {
      const deltaY = startY - moveEvent.clientY;
      const sensitivity = 200; 
      const deltaNorm = deltaY / sensitivity;
      const newNorm = Math.max(0, Math.min(1, startNorm + deltaNorm));

      // UI 즉시 업데이트 (React State)
      const realVal = isLog ? linearToLog(newNorm, min, max) : min + newNorm * (max - min);
      const fixedVal = parseFloat(realVal.toFixed(decimalPlaces));
      setValue(fixedVal);

      // 백엔드 전송
      const normalizedFixedVal = (fixedVal - min) / (max - min);
      sliderStateRef.current?.setNormalisedValue(normalizedFixedVal);
      if (import.meta.env.DEV) {  // Only log in development
        console.log('realVal:', realVal, 'fixedVal:', fixedVal, 'normalizedFixedVal:', normalizedFixedVal);
      }
    };

    const onMouseUp = () => {
      isDragging.current = false;
      window.removeEventListener('mousemove', onMouseMove);
      window.removeEventListener('mouseup', onMouseUp);
    };

    window.addEventListener('mousemove', onMouseMove);
    window.addEventListener('mouseup', onMouseUp);
  };

  const handleManualInput = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter' && sliderStateRef.current) {
      const val = parseFloat((e.target as HTMLInputElement).value);
      if (!isNaN(val)) {
        const clamped = Math.max(min, Math.min(max, val));
        const norm = isLog ? logToLinear(clamped, min, max) : (clamped - min) / (max - min);
        sliderStateRef.current.setNormalisedValue(norm);
        setValue(parseFloat(clamped.toFixed(decimalPlaces)));
      }
      setIsEditing(false);
    }
  };

  // 시각적 표현을 위한 퍼센트 계산
  const percent = isLog ? logToLinear(value, min, max) : (value - min) / (max - min);
  const rotation = percent * 270 - 135;

  return (
    <div className="flex flex-col items-center gap-3">
      <span className="text-[10px] font-black tracking-widest text-cyan-500 uppercase select-none">{label}</span>
      
      <div 
        ref={knobRef}
        onMouseDown={onMouseDown}
        className="relative w-28 h-28 rounded-full bg-slate-900 shadow-[5px_5px_15px_#050505,-5px_-5px_15px_#1a1a1a] flex items-center justify-center cursor-ns-resize group"
      >
        {/* Progress Ring (SVG) */}
        <svg className="absolute w-full h-full" viewBox="0 0 100 100" style={{ transform: `rotate(-225deg)` }}>
          <circle cx="50" cy="50" r="45" fill="none" stroke="#1e293b" strokeWidth="4" />
          <circle 
            cx="50" cy="50" r="45" fill="none" stroke="#06b6d4" strokeWidth="4"
            strokeDasharray={`${211.5 * percent} ${282.7 - 211.5 * percent}`}
            strokeLinecap="round"
            className="transition-none drop-shadow-[0_0_5px_#06b6d4]"
          />
        </svg>

        {/* Knob Face */}
        <div className="w-20 h-20 rounded-full bg-gradient-to-br from-slate-800 to-slate-950 shadow-lg flex flex-col items-center justify-center relative">
          <div className="absolute inset-0 transition-none" style={{ transform: `rotate(${rotation}deg)` }}>
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
              className="text-lg font-black text-slate-100 cursor-text tracking-tighter select-none"
            >
              {value}
            </span>
          )}
          <span className="text-[8px] font-bold text-slate-500 select-none">{unit}</span>
        </div>
      </div>
    </div>
  );
};

export default function LPFUI() {
  const [isBypassed, setIsBypassed] = useState(false);

  return (
    <div className="w-[480px] h-[320px] bg-black bg-[radial-gradient(circle_at_center,_#111_0%,_#000_100%)] flex flex-col items-center justify-between p-6 overflow-hidden font-sans border border-slate-800 select-none">
      <div className="w-full flex justify-between items-center border-b border-cyan-900/30 pb-2">
        <h1 className="text-2xl font-black tracking-tighter text-transparent bg-clip-text bg-gradient-to-r from-cyan-400 to-blue-600 drop-shadow-[0_0_10px_rgba(34,211,238,0.4)]">
          SimpleBiquadLPF
        </h1>
        <Button
          type="button"
          variant={isBypassed ? 'outline' : 'default'}
          size="sm"
          onClick={() => {
            const nextBypassState = !isBypassed;
            setIsBypassed(nextBypassState);
            // sendParamToJuce('bypass', nextBypassState ? 1 : 0);
          }}
          className={`h-8 w-24 px-3 text-[10px] font-black tracking-[0.2em] uppercase transition-all ${
            isBypassed
              ? 'border-slate-700 bg-slate-900 text-white hover:bg-slate-800'
              : 'border-cyan-300 bg-slate-900 text-white shadow-[0_0_12px_rgba(34,211,238,0.25)] hover:bg-slate-800'
          }`}
        >
          {isBypassed ? 'Bypass' : 'Active'}
        </Button>
      </div>

      {/* Control Section */}
      <div className="flex gap-16 items-center flex-1">
        <Knob 
          label="Cutoff" 
          paramId="freqHz"
          min={20} 
          max={22050} 
          initialValue={1000} 
          unit="Hz" 
          isLog={true}
        />
        <Knob 
          label="Resonance" 
          paramId="resonance"
          min={0.0} 
          max={12.0} 
          initialValue={0.7} 
          unit="dB" 
          decimalPlaces={1}
        />
      </div>

      {/* Footer Decoration */}
      <div className="w-full flex justify-between text-[8px] font-mono text-slate-600 tracking-[0.3em] uppercase">
        <span>2026 Jihoaudio</span>
        <span>ahnjiho.com</span>
      </div>
    </div>
  );
}