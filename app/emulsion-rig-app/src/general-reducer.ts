import { createSlice, PayloadAction } from "@reduxjs/toolkit";
import { EmulsionEvent } from "./program"
import {v4 as uuid4} from 'uuid'
import { mapToObj } from "remeda";
import { RootState } from "./state";

type StateShape = {
    events: {[key: string]: EmulsionEvent};
    activeEventId: string | null;
}

const initialState: StateShape = {
    events: {},
    activeEventId: null,
}

export const generalSlice = createSlice({
    name: 'general',
    initialState,
    reducers: {
        init: (state, action: PayloadAction<{events: EmulsionEvent[]}>) => 
            {
                const newObj = {...initialState, events: mapToObj(action.payload.events, event => [uuid4(), event])};
                return newObj;
            },
        chooseEvent: (state, action: PayloadAction<{eventId: string}>) => {
            state.activeEventId = action.payload.eventId;
        },
        
    }
})
export const generalActions = generalSlice.actions;

export const selectActiveEventId = (state: RootState) => state[generalSlice.name].activeEventId;
export const selectAllEvents = (state: RootState) => Object.values(state[generalSlice.name].events);
export const selectActiveEvent = (state: RootState) => selectAllEvents(state).find(event => event.eventId === selectActiveEventId(state));
