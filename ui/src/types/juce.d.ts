export {};

declare global {
  interface Window {
    __juce_backend?: {
      callNativeFunction: (functionName: string, args: any[]) => void;
    };
  }
}