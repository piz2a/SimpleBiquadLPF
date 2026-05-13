import React, { useRef } from 'react';

// JUCE 통신용 스로틀 유틸리티 (16ms = 약 60fps)
function useThrottle(callback: (...args: any[]) => void, delay: number) {
  const lastCall = useRef<number>(0);
  return (...args: any[]) => {
    const now = Date.now();
    if (now - lastCall.current >= delay) {
      lastCall.current = now;
      callback(...args);
    }
  };
}

// 로그 스케일 변환 유틸리티
function linearToLogarithmic(linearValue: number, min: number, max: number): number {
  // linearValue: 0-1 사이의 정규화된 값
  const logMin = Math.log(min);
  const logMax = Math.log(max);
  return Math.exp(logMin + linearValue * (logMax - logMin));
}

function logarithmicToLinear(frequency: number, min: number, max: number): number {
  // frequency를 0-1 사이의 정규화된 값으로 변환
  const logMin = Math.log(min);
  const logMax = Math.log(max);
  return (Math.log(frequency) - logMin) / (logMax - logMin);
}

export default function NeumorphicOneKnob() {
  const MIN_FREQ = 20;
  const MAX_FREQ = 22050;
  
  const freqRef = useRef(1000);
  const knobHandleRef = useRef<HTMLDivElement>(null);
  const freqTextRef = useRef<HTMLSpanElement>(null);
  const ringRef = useRef<SVGCircleElement>(null);

  const sendParamToJuce = useThrottle((paramId: string, value: number) => {
    if (window.__juce_backend) {
      window.__juce_backend.callNativeFunction("updateParameter", [paramId, value]);
    } else {
      console.log(`[JUCE] ${paramId}: ${value}`);
    }
  }, 16);

  const handleInput = (e: React.FormEvent<HTMLInputElement>) => {
    const target = e.target as HTMLInputElement;
    const inputValue = Number(target.value);
    
    // inputValue: 0-100 (슬라이더 범위를 0-100으로 정규화)
    const normalized = inputValue / 100;
    
    // 로그 스케일로 주파수 계산
    const frequency = linearToLogarithmic(normalized, MIN_FREQ, MAX_FREQ);
    freqRef.current = frequency;

    // 1. 노브 회전 (로그 스케일 기반)
    if (knobHandleRef.current) {
      const rotation = normalized * 270 - 135;
      knobHandleRef.current.style.transform = `rotate(${rotation}deg)`;
    }

    // 2. SVG 프로그레스 링 (로그 스케일 기반)
    if (ringRef.current) {
      const circumference = 2 * Math.PI * 45;
      const offset = circumference - (circumference * normalized);
      ringRef.current.style.strokeDashoffset = String(offset);
    }

    // 3. 텍스트 - 정수 또는 소수점 표시
    if (freqTextRef.current) {
      const displayValue = frequency < 1000 
        ? Math.round(frequency).toString()
        : frequency.toFixed(1);
      freqTextRef.current.innerText = displayValue;
    }

    sendParamToJuce("freqHz", frequency);
  };

  return (
    <div className="w-[480px] h-[320px] bg-[#e0e5ec] flex items-center justify-between px-10 overflow-hidden font-sans text-[#444] select-none">
      
      {/* Left Section: Info */}
      <div className="flex flex-col gap-1 w-24">
        <h1 className="text-[10px] font-black tracking-[0.2em] text-[#7a8da1] uppercase">Simple Filter</h1>
        <div className="w-6 h-[2px] bg-emerald-400 shadow-[0_0_5px_#34d399] mb-4" />
        <div className="text-[9px] font-bold text-[#a3b1c6] leading-tight">
          S6 SELECTIVE<br/>CORE V2
        </div>
      </div>

      {/* Center Section: Main Knob */}
      <div className="relative w-44 h-44 flex items-center justify-center">
        {/* Outer Shadow (Sunken) */}
        <div className="absolute inset-0 rounded-full shadow-[inset_4px_4px_8px_#bebebe,inset_-4px_-4px_8px_#ffffff]" />
        
        {/* SVG Progress Ring */}
        <svg className="absolute w-full h-full -rotate-90 pointer-events-none" viewBox="0 0 100 100">
          <circle 
            ref={ringRef}
            cx="50" cy="50" r="45" 
            fill="transparent" 
            stroke="#10b981" 
            strokeWidth="4" 
            strokeDasharray={2 * Math.PI * 45}
            strokeDashoffset={2 * Math.PI * 45 * (1 - logarithmicToLinear(freqRef.current, MIN_FREQ, MAX_FREQ))}
            strokeLinecap="round"
            className="transition-none"
          />
        </svg>

        {/* Knob Body (Raised) */}
        <div className="w-32 h-32 rounded-full bg-[#e0e5ec] shadow-[6px_6px_12px_#bebebe,-6px_-6px_12px_#ffffff] flex items-center justify-center relative">
          <div 
            ref={knobHandleRef}
            className="absolute inset-0 transition-none"
            style={{ transform: `rotate(${logarithmicToLinear(freqRef.current, MIN_FREQ, MAX_FREQ) * 270 - 135}deg)` }}
          >
            <div className="absolute top-3 left-1/2 -translate-x-1/2 w-2 h-2 bg-emerald-500 rounded-full shadow-[0_0_5px_#10b981]" />
          </div>

          <div className="flex flex-col items-center pointer-events-none">
            <span ref={freqTextRef} className="text-2xl font-black text-[#4d5d6d]">
              {freqRef.current < 1000 ? Math.round(freqRef.current) : freqRef.current.toFixed(1)}
            </span>
            <span className="text-[9px] font-bold text-[#a3b1c6] tracking-widest uppercase">Hz</span>
          </div>
        </div>

        {/* Vertical Invisible Control Overlay */}
        <input 
          type="range" 
          min="0" max="100" 
          defaultValue={logarithmicToLinear(freqRef.current, MIN_FREQ, MAX_FREQ) * 100}
          onInput={handleInput}
          className="absolute inset-0 w-full h-full opacity-0 cursor-ns-resize z-10"
          style={{ WebkitAppearance: 'slider-vertical' } as React.CSSProperties}
        />
      </div>

      {/* Right Section: Visualizer Placeholder */}
      <div className="w-24 flex flex-col items-end gap-3">
        <div className="flex gap-1 h-12 items-end">
          {[0.4, 0.7, 1, 0.8, 0.5].map((h, i) => (
            <div key={i} className="w-1 bg-[#c0c9d6] rounded-full" style={{ height: `${h * 100}%` }} />
          ))}
        </div>
        <div className="px-2 py-1 rounded bg-[#e0e5ec] shadow-[inset_2px_2px_4px_#bebebe,inset_-2px_-2px_4px_#ffffff] text-[8px] font-mono text-[#7a8da1]">
          ACTIVE_DSP
        </div>
      </div>
    </div>
  );
}