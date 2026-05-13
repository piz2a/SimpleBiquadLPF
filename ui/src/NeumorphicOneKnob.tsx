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

export default function NeumorphicOneKnob() {
  // 가상 DOM 리렌더링 없이 값 관리
  const freqRef = useRef(1000);
  
  // DOM 직접 조작을 위한 레퍼런스
  const knobHandleRef = useRef<HTMLDivElement>(null);
  const freqTextRef = useRef<HTMLSpanElement>(null);
  const ringRef = useRef<SVGCircleElement>(null);

  // JUCE 백엔드 전송 (스로틀링 적용)
  const sendParamToJuce = useThrottle((paramId: string, value: number) => {
    if (window.__juce_backend) {
      window.__juce_backend.callNativeFunction("updateParameter", [paramId, value]);
    } else {
      console.log(`[JUCE] ${paramId}: ${value}`);
    }
  }, 16);

  const handleInput = (e: React.FormEvent<HTMLInputElement>) => {
    const target = e.target as HTMLInputElement;
    const val = Number(target.value);
    freqRef.current = val;

    // 1. 노브 회전 (Transform 직접 조작)
    if (knobHandleRef.current) {
      const rotation = (val / 20000) * 270 - 135; // -135도 ~ +135도 범위
      knobHandleRef.current.style.transform = `rotate(${rotation}deg)`;
    }

    // 2. SVG 프로그레스 링 업데이트
    if (ringRef.current) {
      const circumference = 2 * Math.PI * 45;
      const offset = circumference - (circumference * (val / 20000));
      ringRef.current.style.strokeDashoffset = String(offset);
    }

    // 3. 텍스트 업데이트
    if (freqTextRef.current) {
      freqTextRef.current.innerText = val >= 1000 
        ? `${(val / 1000).toFixed(2)} kHz` 
        : `${Math.round(val)} Hz`;
    }

    sendParamToJuce("freqHz", val);
  };

  return (
    <div className="w-[400px] h-[450px] bg-[#e0e5ec] flex flex-col items-center justify-center p-8 rounded-[40px] shadow-[20px_20px_60px_#bebebe,-20px_-20px_60px_#ffffff] font-sans text-[#444]">
      
      {/* Header */}
      <div className="mb-10 text-center">
        <h1 className="text-sm font-black tracking-[0.2em] text-[#7a8da1] uppercase">Simple Filter</h1>
        <div className="w-1.5 h-1.5 bg-emerald-400 rounded-full mx-auto mt-2 shadow-[0_0_8px_#34d399]" />
      </div>

      {/* Neumorphic Knob Section */}
      <div className="relative w-48 h-48 flex items-center justify-center">
        {/* Outer Ring Shadow (Sunken) */}
        <div className="absolute inset-0 rounded-full shadow-[inset_6px_6px_12px_#bebebe,inset_-6px_-6px_12px_#ffffff]" />
        
        {/* SVG Progress Ring */}
        <svg className="absolute w-full h-full -rotate-90 pointer-events-none" viewBox="0 0 100 100">
          <circle 
            ref={ringRef}
            cx="50" cy="50" r="45" 
            fill="transparent" 
            stroke="#10b981" 
            strokeWidth="3" 
            strokeDasharray={2 * Math.PI * 45}
            strokeDashoffset={2 * Math.PI * 45 * (1 - freqRef.current / 20000)}
            strokeLinecap="round"
            className="transition-none"
          />
        </svg>

        {/* Main Knob Body (Raised) */}
        <div className="w-36 h-36 rounded-full bg-[#e0e5ec] shadow-[8px_8px_16px_#bebebe,-8px_-8px_16px_#ffffff] flex items-center justify-center relative">
          
          {/* Indicator Dot (Rotates) */}
          <div 
            ref={knobHandleRef}
            className="absolute inset-0 transition-none"
            style={{ transform: `rotate(${(freqRef.current / 20000) * 270 - 135}deg)` }}
          >
            <div className="absolute top-4 left-1/2 -translate-x-1/2 w-2 h-2 bg-emerald-500 rounded-full shadow-[0_0_5px_#10b981]" />
          </div>

          {/* Value Display */}
          <div className="flex flex-col items-center pointer-events-none">
            <span ref={freqTextRef} className="text-xl font-bold text-[#4d5d6d]">
              {freqRef.current} Hz
            </span>
            <span className="text-[10px] font-bold text-[#a3b1c6] mt-1 tracking-widest uppercase">Cutoff</span>
          </div>
        </div>

        {/* Invisible Range Input for Interaction */}
        <input 
          type="range" 
          min="20" 
          max="20000" 
          step="1"
          defaultValue={freqRef.current}
          onInput={handleInput}
          className="absolute inset-0 w-full h-full opacity-0 cursor-pointer z-10"
        />
      </div>

      {/* Footer Info */}
      <div className="mt-12 flex flex-col items-center gap-2">
        <div className="px-4 py-1.5 rounded-full bg-[#e0e5ec] shadow-[inset_3px_3px_6px_#bebebe,inset_-3px_-3px_6px_#ffffff] text-[10px] font-bold text-[#7a8da1]">
          S6 SELECTIVE ARCHITECTURE
        </div>
        <p className="text-[9px] text-[#a3b1c6] font-medium italic">Precision Audio Control v2.0</p>
      </div>
    </div>
  );
}