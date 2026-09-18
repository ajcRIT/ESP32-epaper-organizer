
const COLOR_ID_MAP = {
"1": "#a4bdfc", // Blue
"2": "#7ae7bf", // Green
"3": "#dbadff", // Purple
"4": "#ff887c", // Red
"5": "#fbd75b", // Yellow
"6": "#ffb878", // Orange
"7": "#46d6db", // Cyan
"8": "#e1e1e1", // Gray
"9": "#5484ed", // Bold Blue
"10": "#51b749", // Bold Green
"11": "#dc2127" // Bold Red
};

function doGet() {

  const CALENDAR_IDS = ['ajc3023@g.rit.edu',
                        'c_096b31195964106a69bc43d61bbfce992b2e49453d9297644d46c39440c81c89@group.calendar.google.com',
                        'c_71924d890d388904b5433cad6d8f72a92b90b2ec0f8f2ef8126fe5e92fb2254a@group.calendar.google.com',
                        'c_de6813caddc6252e5f81a15b8f1f9b7cc92f5ac7048701cb598d6e171b215b70@group.calendar.google.com'];
  const DAYS_IN_FUTURE = 7;
  const FALLBACK_COLOR = "#888888"; // grey

  const now = new Date();
  const futureDate = new Date();
  futureDate.setDate(now.getDate() + DAYS_IN_FUTURE);

  let combinedEvents = [];

  CALENDAR_IDS.forEach(id => {
    try {
      const calendar = CalendarApp.getCalendarById(id);
      if (!calendar) {
        Logger.log('Error: Calendar not found for ID: ' + id);
        return ContentService.createTextOutput(JSON.stringify({ error: "Calendar not found" })).setMimeType(ContentService.MimeType.JSON);
      }

      const events = calendar.getEvents(now, futureDate);

      const processedEvents = events.map(event => {
        let eventColorId = event.getColor();
        let finalHexColor;

        if (eventColorId) {
          // The event has a specific palette color. Look up its hex code.
          finalHexColor = COLOR_ID_MAP[eventColorId] || calendar.getColor() || FALLBACK_COLOR;
        } else {
          // The event uses the calendar's default color.
          finalHexColor = calendar.getColor() || FALLBACK_COLOR;
        }

        return {
          title: event.getTitle(),
          startTime: event.getStartTime().toISOString(),
          endTime: event.getEndTime().toISOString(),
          color: finalHexColor // This is now guaranteed to be a hex code
        };
      });

      combinedEvents = combinedEvents.concat(processedEvents);

    } catch (e) {
      Logger.log('An error occurred: ' + e.message);
      return ContentService.createTextOutput(JSON.stringify({ error: e.message })).setMimeType(ContentService.MimeType.JSON);
    }
  });
    combinedEvents.sort((a,b)=> new Date(a.startTime)-new Date(b.startTime));
    return ContentService.createTextOutput(JSON.stringify(combinedEvents)).setMimeType(ContentService.MimeType.JSON);
}
