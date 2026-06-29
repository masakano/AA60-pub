import React, { useEffect, useRef, useState } from 'react';
import { createRoot } from 'react-dom/client';
import './styles.css';

const apiUrl = import.meta.env.VITE_WAN21_API ?? `${window.location.protocol}//${window.location.hostname}:8000`;

function waitIceGatheringComplete(pc) {
  if (pc.iceGatheringState === 'complete') {
    return Promise.resolve();
  }
  return new Promise((resolve) => {
    const onStateChange = () => {
      if (pc.iceGatheringState === 'complete') {
        pc.removeEventListener('icegatheringstatechange', onStateChange);
        resolve();
      }
    };
    pc.addEventListener('icegatheringstatechange', onStateChange);
  });
}

function describeInput(health) {
  if (!health) {
    return 'offline';
  }
  if (health.source === 'shm') {
    return health.shm_exists ? `shm ${health.output_width ?? health.width}x${health.output_height ?? health.height}` : 'shm missing';
  }
  if (health.source === 'h264') {
    return health.h264_exists ? `h264 ${health.width}x${health.height}` : 'h264 missing';
  }
  return `${health.width}x${health.height}`;
}

function App() {
  const videoRef = useRef(null);
  const pcRef = useRef(null);
  const [status, setStatus] = useState('idle');
  const [health, setHealth] = useState(null);
  const [streamEnabled, setStreamEnabled] = useState(true);
  const [streamFps, setStreamFps] = useState(60);
  const [controlStatus, setControlStatus] = useState('idle');

  const stop = () => {
    if (pcRef.current) {
      pcRef.current.close();
      pcRef.current = null;
    }
    if (videoRef.current) {
      videoRef.current.srcObject = null;
    }
    setStatus('idle');
  };

  const start = async () => {
    stop();
    setStatus('connecting');

    const pc = new RTCPeerConnection({ iceServers: [] });
    pcRef.current = pc;
    pc.addTransceiver('video', { direction: 'recvonly' });

    pc.ontrack = (event) => {
      if (videoRef.current) {
        videoRef.current.srcObject = event.streams[0];
        videoRef.current.play().catch(() => {});
      }
    };

    pc.onconnectionstatechange = () => {
      setStatus(pc.connectionState);
    };

    const offer = await pc.createOffer();
    await pc.setLocalDescription(offer);
    await waitIceGatheringComplete(pc);

    const response = await fetch(`${apiUrl}/offer`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(pc.localDescription),
    });
    if (!response.ok) {
      throw new Error(await response.text());
    }
    const answer = await response.json();
    await pc.setRemoteDescription(answer);
  };

  useEffect(() => {
    fetch(`${apiUrl}/health`)
      .then((response) => response.json())
      .then((nextHealth) => {
        setHealth(nextHealth);
        if (nextHealth?.control) {
          setStreamEnabled(Boolean(nextHealth.control.enable));
          setStreamFps(nextHealth.control.fps || 60);
        }
      })
      .catch(() => setHealth(null));
    return stop;
  }, []);

  const onStart = async () => {
    try {
      await start();
    } catch (error) {
      console.error(error);
      setStatus('error');
      stop();
    }
  };

  const updateControl = async (nextValues = {}) => {
    const payload = {
      enable: nextValues.enable ?? streamEnabled,
      fps: nextValues.fps ?? streamFps,
    };
    setControlStatus('sending');
    try {
      const response = await fetch(`${apiUrl}/control`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload),
      });
      if (!response.ok) {
        throw new Error(await response.text());
      }
      const body = await response.json();
      setHealth((current) => ({ ...(current ?? {}), control: body.control }));
      setStreamEnabled(Boolean(body.control.enable));
      setStreamFps(body.control.fps || payload.fps);
      setControlStatus(`serial ${body.control.serial}`);
    } catch (error) {
      console.error(error);
      setControlStatus('error');
    }
  };

  const onEnableChange = (event) => {
    const enable = event.target.checked;
    setStreamEnabled(enable);
    updateControl({ enable });
  };

  const onFpsChange = (event) => {
    setStreamFps(Number(event.target.value));
  };

  const onControlSubmit = (event) => {
    event.preventDefault();
    updateControl();
  };

  return (
    <main className="appShell">
      <header className="toolbar">
        <div className="brand">WAN21 Monitor</div>
        <div className="metrics">
          <span>{status}</span>
          <span>{health?.source ?? 'offline'}</span>
          <span>{describeInput(health)}</span>
        </div>
        <form className="controlPanel" onSubmit={onControlSubmit}>
          <label className="toggleControl">
            <input type="checkbox" checked={streamEnabled} onChange={onEnableChange} />
            <span>Stream</span>
          </label>
          <label className="fpsControl">
            <span>FPS</span>
            <input type="number" min="1" max="240" step="1" value={streamFps} onChange={onFpsChange} />
          </label>
          <button type="submit" disabled={controlStatus === 'sending'}>
            Apply
          </button>
          <span className="controlStatus">{controlStatus}</span>
        </form>
        <div className="actions">
          <button onClick={onStart} disabled={status === 'connecting' || status === 'connected'}>
            Connect
          </button>
          <button onClick={stop} disabled={status === 'idle'}>
            Stop
          </button>
        </div>
      </header>
      <section className="stage">
        <video ref={videoRef} autoPlay playsInline muted />
      </section>
    </main>
  );
}

createRoot(document.getElementById('root')).render(<App />);
