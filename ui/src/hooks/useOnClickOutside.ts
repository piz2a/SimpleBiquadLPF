import { useEffect, type RefObject } from 'react';

export default function useOnClickOutside<T extends HTMLElement>(
  ref: RefObject<T | null>[],
  handler: (event: MouseEvent | TouchEvent) => void
) {
  useEffect(() => {
    const listener = (event: MouseEvent | TouchEvent) => {
      // ref가 없거나, 클릭된 타겟이 ref 안에 포함되어 있다면 아무것도 하지 않음
      // for multiple refs, check if the event target is inside any of the refs
      if (ref.some(r => r.current && r.current.contains(event.target as Node))) {
        return;
      }
      
      // 영역 밖을 클릭한 경우 핸들러 실행
      handler(event);
    };

    // 전역 document에 이벤트 리스너 등록
    document.addEventListener('mousedown', listener);
    document.addEventListener('touchstart', listener);

    // 컴포넌트 언마운트 시 리스너 제거 (메모리 누수 방지)
    return () => {
      document.removeEventListener('mousedown', listener);
      document.removeEventListener('touchstart', listener);
    };
  }, [ref, handler]); // ref나 handler가 바뀌면 리스너 재설정
}