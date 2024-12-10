import { useEffect, useState } from 'react'
import reactLogo from './assets/react.svg'
import viteLogo from '/vite.svg'
import './App.css'
import * as R from 'remeda'
import { EmulsionProgram } from './program'
import {Provider} from 'react-redux';
import { store } from './state'
import { useAppDispatch, useAppSelector } from './use-hooks'
import { generalActions, selectActiveEvent, selectAllEvents } from './general-reducer'

const secondSize = 3;

const totalDuration = 180;

const totalMinutes = totalDuration / 60;

const maxSpeed = 100;
const speedWidth = 50;

const SAMPLE_PROGRAM: EmulsionProgram = {
  events: [
    {eventId: '1', offset: 1, type: "heading", text: "Preparing..."},
    {eventId: '2', offset: 5, type: "heading", text: "Pour the lime"},
    {eventId: '3', offset: 30, type: "heading", text: "Pour the coconut"},
    {eventId: '4', offset: 50, type: "heading", text: "Shake it all up"},
    {eventId: 's1', offset: 1, type: "ramp_speed", value: 0.0, seconds: 0},
    {eventId: 's2', offset: 50, type: "ramp_speed", value: 0.5, seconds: 20},
    {eventId: 's3', offset: 90, type: "ramp_speed", value: 0.2, seconds: 15},
  ]
}
console.log(JSON.stringify(SAMPLE_PROGRAM))

function App() {
  const dispatch = useAppDispatch();
  useEffect(()=>{dispatch(generalActions.init({events: SAMPLE_PROGRAM.events}))}, [dispatch])
  const events = useAppSelector(selectAllEvents);
  const activeEvent = useAppSelector(selectActiveEvent);

  const speedAlteringEvents = events.filter(event => event.type === "ramp_speed");
  speedAlteringEvents.sort((a,b) => a.offset - b.offset)

  let previousValue = 0;
  const speedPoints = speedAlteringEvents.map(
    event => {
      const x1 = previousValue * speedWidth;
      const y1 = event.offset * secondSize;

      const x2 = event.value * speedWidth;
      const y2 = (event.offset + event.seconds) * secondSize;

      previousValue = event.value;

      return `${x1},${y1} ${x2},${y2}`;
    }
  );
  console.log(speedAlteringEvents, speedPoints);
  
  return (
    <>
      <div className="EmulsionApp">
        <div className="EmulsionApp__main">
          
          </div>  
        <div className="EmulsionApp__timeline">
          <svg width={100} height={500} viewBox='0 0 100 500' fill={"yellow"} >

            <polyline className="Timeline__speedline" points={speedPoints.join(" ")} />
            {R.range(0, totalDuration).filter(i => i % 1 == 0).map(
              i => (<line key={i} x1={5} x2={50} y1={i*secondSize} y2={i*secondSize} className="Timeline__second_minor" />))}

            {R.range(0, totalDuration).filter(i => i % 5 == 0).map(
              i => (<line key={i} x1={5} x2={50} y1={i*secondSize} y2={i*secondSize} className="Timeline__second_major" />))}

          {R.range(0, totalMinutes).map(
              i => (<line key={i} x1={5} x2={50} y1={i*60*secondSize} y2={i*60*secondSize} className="Timeline__minute" />))}
              
              {(events).map(
                event => {
                  const y = event.offset * secondSize;
                  switch(event.type) {
                    case "heading":
                      return <g key={event.eventId} className="Timeline__event" data-active={activeEvent?.eventId === event.eventId} ><line x1={0} x2={45} y1={y} y2={y} /> <text x={0} y={y+5} onClick={()=>dispatch(generalActions.chooseEvent({eventId: event.eventId}))}>{event.text}</text></g>
                    default:
                      return null;
                  }
                }
              )}


          </svg>
        </div>  
      </div>
      
    </>
  )
}

function AppContainer() {

  return <Provider store={store}>
    <App />
  </Provider>;
}

export default AppContainer
