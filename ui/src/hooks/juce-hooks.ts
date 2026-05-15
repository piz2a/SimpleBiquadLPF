import * as Juce from '../juce';
import { useState, useEffect, useRef } from 'react';
import { linearToLog, logToLinear } from '../utils/scale-transformation';

export function useJuceSlider(paramId: string, min: number, max: number, isLog = false, decimalPlaces = 0, initialValue = 0) {
    const [value, setValue] = useState(initialValue);
    const [isEditing, setIsEditing] = useState(false);

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

    return {
        value,
        setValue,
        sliderStateRef,
        isDragging,
        isEditing,
        setIsEditing,
        handleManualInput,
    };
}

export function useJuceKnob(paramId: string, min: number, max: number, isLog = false, decimalPlaces = 0, initialValue = 0) {
    const {value, setValue, sliderStateRef, isDragging, isEditing, setIsEditing, handleManualInput} = useJuceSlider(paramId, min, max, isLog, decimalPlaces, initialValue);

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

    return {
        value,
        setValue,
        sliderStateRef,
        isDragging,
        isEditing,
        setIsEditing,
        handleManualInput,
        onMouseDown,  // drag handling for knob
    };
}

export function useJuceToggle(paramId: string) {
    const [value, setValue] = useState(false);
    const toggleStateRef = useRef<Juce.ToggleState>(null);

    // Load ToggleState from JUCE and set up listener
    useEffect(() => {
        const state = Juce.getToggleState(paramId);
        console.log(`✅ ToggleState for ${paramId} obtained:`, state);
        toggleStateRef.current = state;

        const listener = () => {
            const toggleValue = state.getValue();
            console.log(`🔔 ${paramId} toggle value from JUCE:`, toggleValue);
            setValue(toggleValue);
        };

        const listenerId = state.valueChangedEvent.addListener(listener);
        listener(); // 초기 상태 동기화

        // Cleanup
        return () => state.valueChangedEvent.removeListener(listenerId);
    }, [paramId]);

    const handleToggle = () => {
        if (toggleStateRef.current) {
            const newValue = !value;
            toggleStateRef.current.setValue(newValue);
            setValue(newValue);
        }
    };

    return {
        value,
        setValue,
        toggleStateRef,
        handleToggle,
    };
}

export function useJuceComboBox(paramId: string) {
}