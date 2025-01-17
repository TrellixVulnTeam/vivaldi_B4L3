// Copyright (c) 2017 Vivaldi Technologies AS. All rights reserved

#include "extensions/api/calendar/calendar_api.h"

#include <string>
#include <vector>

#include "base/lazy_instance.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/profiles/profile.h"
#include "content/public/browser/browser_context.h"
#include "extensions/browser/event_router.h"
#include "extensions/schema/calendar.h"

#include "calendar/calendar_model_observer.h"
#include "calendar/calendar_service.h"
#include "calendar/calendar_service_factory.h"

using calendar::CalendarService;
using calendar::CalendarServiceFactory;

namespace extensions {

using vivaldi::calendar::Calendar;

namespace OnEventCreated = vivaldi::calendar::OnEventCreated;
namespace OnEventRemoved = vivaldi::calendar::OnEventRemoved;
namespace OnEventChanged = vivaldi::calendar::OnEventChanged;

typedef std::vector<vivaldi::calendar::CalendarEvent> EventList;
typedef std::vector<vivaldi::calendar::Calendar> CalendarList;

double MilliSecondsFromTime(const base::Time& time) {
  return 1000 * time.ToDoubleT();
}

CalendarEvent GetEventItem(const calendar::EventRow& row) {
  CalendarEvent event_item;
  event_item.id = base::Int64ToString(row.id());
  event_item.description.reset(
      new std::string(base::UTF16ToUTF8(row.description())));
  event_item.title.reset(new std::string(base::UTF16ToUTF8(row.title())));
  event_item.start.reset(new double(MilliSecondsFromTime(row.start())));
  event_item.end.reset(new double(MilliSecondsFromTime(row.end())));
  return event_item;
}

Calendar GetCalendarItem(const calendar::CalendarRow& row) {
  Calendar calendar;
  calendar.id = row.id();
  calendar.name.reset(new std::string(base::UTF16ToUTF8(row.name())));
  calendar.description.reset(
      new std::string(base::UTF16ToUTF8(row.description())));
  calendar.orderindex.reset(new int(row.orderindex()));
  return calendar;
}

CalendarEventRouter::CalendarEventRouter(Profile* profile)
    : browser_context_(profile),
      model_(CalendarServiceFactory::GetForProfile(profile)) {
  model_->AddObserver(this);
}

CalendarEventRouter::~CalendarEventRouter() {
  model_->RemoveObserver(this);
}

void CalendarEventRouter::ExtensiveCalendarChangesBeginning(
    CalendarService* model) {}

void CalendarEventRouter::ExtensiveCalendarChangesEnded(
    CalendarService* model) {}

void CalendarEventRouter::OnEventCreated(CalendarService* service,
                                         const calendar::EventRow& row) {
  CalendarEvent createdEvent = GetEventItem(row);
  std::unique_ptr<base::ListValue> args = OnEventCreated::Create(createdEvent);
  DispatchEvent(OnEventCreated::kEventName, std::move(args));
}

void CalendarEventRouter::OnEventDeleted(CalendarService* service,
                                         const calendar::EventRow& row) {
  CalendarEvent deletedEvent = GetEventItem(row);
  std::unique_ptr<base::ListValue> args = OnEventRemoved::Create(deletedEvent);
  DispatchEvent(OnEventCreated::kEventName, std::move(args));
}

void CalendarEventRouter::OnEventChanged(CalendarService* service,
                                         const calendar::EventRow& row) {
  CalendarEvent changedEvent = GetEventItem(row);
  std::unique_ptr<base::ListValue> args = OnEventChanged::Create(changedEvent);
  DispatchEvent(OnEventChanged::kEventName, std::move(args));
}

// Helper to actually dispatch an event to extension listeners.
void CalendarEventRouter::DispatchEvent(
    const std::string& event_name,
    std::unique_ptr<base::ListValue> event_args) {
  EventRouter* event_router = EventRouter::Get(browser_context_);
  if (event_router) {
    event_router->BroadcastEvent(base::WrapUnique(
        new extensions::Event(extensions::events::VIVALDI_EXTENSION_EVENT,
                              event_name, std::move(event_args))));
  }
}

void BroadcastCalendarEvent(const std::string& eventname,
                            std::unique_ptr<base::ListValue> args,
                            content::BrowserContext* context) {
  std::unique_ptr<extensions::Event> event(
      new extensions::Event(extensions::events::VIVALDI_EXTENSION_EVENT,
                            eventname, std::move(args), context));
  EventRouter* event_router = EventRouter::Get(context);
  if (event_router) {
    event_router->BroadcastEvent(std::move(event));
  }
}

CalendarAPI::CalendarAPI(content::BrowserContext* context)
    : browser_context_(context) {
  EventRouter* event_router = EventRouter::Get(browser_context_);
  event_router->RegisterObserver(this, OnEventCreated::kEventName);
  event_router->RegisterObserver(this, OnEventRemoved::kEventName);
  event_router->RegisterObserver(this, OnEventChanged::kEventName);
}

CalendarAPI::~CalendarAPI() {}

void CalendarAPI::Shutdown() {
  calendar_event_router_.reset();
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

static base::LazyInstance<
    BrowserContextKeyedAPIFactory<CalendarAPI>>::DestructorAtExit g_factory =
    LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<CalendarAPI>* CalendarAPI::GetFactoryInstance() {
  return g_factory.Pointer();
}

void CalendarAPI::OnListenerAdded(const EventListenerInfo& details) {
  calendar_event_router_.reset(
      new CalendarEventRouter(Profile::FromBrowserContext(browser_context_)));
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

std::unique_ptr<CalendarEvent> CreateVivaldiEvent(
    const calendar::EventResult& event) {
  std::unique_ptr<CalendarEvent> cal_event(new CalendarEvent());

  cal_event->id = base::Int64ToString(event.id());
  cal_event->calendar_id.reset(
      new std::string(base::Int64ToString(event.calendar_id())));

  cal_event->alarm_id.reset(
      new std::string(base::Int64ToString(event.alarm_id())));

  cal_event->title.reset(new std::string(base::UTF16ToUTF8(event.title())));
  cal_event->description.reset(
      new std::string(base::UTF16ToUTF8(event.description())));
  cal_event->start.reset(new double(MilliSecondsFromTime(event.start())));
  cal_event->end.reset(new double(MilliSecondsFromTime(event.end())));
  cal_event->all_day.reset(new bool(event.all_day()));
  cal_event->is_recurring.reset(new bool(event.is_recurring()));
  cal_event->start_recurring.reset(
      new double(MilliSecondsFromTime(event.start_recurring())));
  cal_event->end_recurring.reset(
      new double(MilliSecondsFromTime(event.end_recurring())));
  cal_event->location.reset(
      new std::string(base::UTF16ToUTF8(event.location())));
  cal_event->url.reset(new std::string(base::UTF16ToUTF8(event.url())));

  return cal_event;
}

CalendarService* CalendarAsyncFunction::GetCalendarService() {
  return CalendarServiceFactory::GetForProfile(GetProfile());
}

CalendarGetAllEventsFunction::~CalendarGetAllEventsFunction() {}

ExtensionFunction::ResponseAction CalendarGetAllEventsFunction::Run() {
  CalendarService* model = CalendarServiceFactory::GetForProfile(GetProfile());

  model->GetAllEvents(
      base::Bind(&CalendarGetAllEventsFunction::GetAllEventsComplete, this),
      &task_tracker_);

  return RespondLater();  // GetAllEventsComplete() will be called
                          // asynchronously.
}

void CalendarGetAllEventsFunction::GetAllEventsComplete(
    std::shared_ptr<calendar::EventQueryResults> results) {
  EventList eventList;

  if (results && !results->empty()) {
    for (calendar::EventQueryResults::EventResultVector::const_iterator
             iterator = results->begin();
         iterator != results->end(); ++iterator) {
      eventList.push_back(std::move(
          *base::WrapUnique((CreateVivaldiEvent(**iterator).release()))));
    }
  }

  Respond(ArgumentList(
      vivaldi::calendar::GetAllEvents::Results::Create(eventList)));
}

Profile* CalendarAsyncFunction::GetProfile() const {
  return Profile::FromBrowserContext(browser_context());
}

base::Time GetTime(double ms_from_epoch) {
  double seconds_from_epoch = ms_from_epoch / 1000.0;
  return (seconds_from_epoch == 0)
             ? base::Time::UnixEpoch()
             : base::Time::FromDoubleT(seconds_from_epoch);
}

ExtensionFunction::ResponseAction CalendarEventCreateFunction::Run() {
  std::unique_ptr<vivaldi::calendar::EventCreate::Params> params(
      vivaldi::calendar::EventCreate::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  calendar::EventRow createEvent;

  base::string16 title;
  title = base::UTF8ToUTF16(params->event.title);

  createEvent.set_title(title);

  if (params->event.description.get()) {
    base::string16 description;
    description = base::UTF8ToUTF16(*params->event.description);
    createEvent.set_description(description);
  }

  if (params->event.start.get()) {
    double start;
    start = *params->event.start.get();
    createEvent.set_start(GetTime(start));
  }

  double end;
  if (params->event.end.get()) {
    end = *params->event.end.get();
    createEvent.set_end(GetTime(end));
  }

  if (params->event.all_day.get()) {
    bool all_day = false;
    end = *params->event.all_day.get();
    createEvent.set_all_day(all_day);
  }

  if (params->event.is_recurring.get()) {
    bool is_recurring = false;
    is_recurring = *params->event.is_recurring.get();
    createEvent.set_is_recurring(is_recurring);
  }

  if (params->event.start_recurring.get()) {
    double start_recurring;
    start_recurring = *params->event.start_recurring.get();
    createEvent.set_start_recurring(GetTime(start_recurring));
  }

  if (params->event.end_recurring.get()) {
    double end_recurring;
    end_recurring = *params->event.end_recurring.get();
    createEvent.set_end_recurring(GetTime(end_recurring));
  }

  base::string16 location;
  if (params->event.location.get()) {
    location = base::UTF8ToUTF16(*params->event.location);
    createEvent.set_location(location);
  }

  base::string16 url;
  if (params->event.url.get()) {
    url = base::UTF8ToUTF16(*params->event.url);
    createEvent.set_url(url);
  }

  CalendarService* model = CalendarServiceFactory::GetForProfile(GetProfile());

  model->CreateCalendarEvent(
      createEvent,
      base::Bind(&CalendarEventCreateFunction::CreateEventComplete, this),
      &task_tracker_);
  return RespondLater();
}

void CalendarEventCreateFunction::CreateEventComplete(
    std::shared_ptr<calendar::CreateEventResult> results) {
  if (!results->success) {
    Respond(Error("Error creating event"));
  } else {
    CalendarEvent ev = GetEventItem(results->createdRow);
    Respond(ArgumentList(
        extensions::vivaldi::calendar::EventCreate::Results::Create(ev)));
  }
}

bool GetIdAsInt64(const base::string16& id_string, int64_t* id) {
  if (base::StringToInt64(id_string, id))
    return true;

  return false;
}

bool GetStdStringAsInt64(const std::string& id_string, int64_t* id) {
  if (base::StringToInt64(id_string, id))
    return true;

  return false;
}

ExtensionFunction::ResponseAction CalendarUpdateEventFunction::Run() {
  std::unique_ptr<vivaldi::calendar::UpdateEvent::Params> params(
      vivaldi::calendar::UpdateEvent::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  calendar::CalendarEvent updatedEvent;

  base::string16 id;
  id = base::UTF8ToUTF16(params->id);
  calendar::EventID eventId;

  if (!GetIdAsInt64(id, &eventId)) {
    return RespondNow(Error("Error. Invalid event id"));
  }

  if (params->changes.calendar_id.get()) {
    calendar::CalendarID calendarId;
    if (!GetStdStringAsInt64(*params->changes.calendar_id, &calendarId)) {
      return RespondNow(Error("Error. Invalid calendar_id"));
    }
    updatedEvent.calendar_id = calendarId;
    updatedEvent.updateFields |= calendar::CALENDAR_ID;
  }

  if (params->changes.alarm_id.get()) {
    calendar::AlarmID alarmId;
    if (!GetStdStringAsInt64(*params->changes.calendar_id, &alarmId)) {
      return RespondNow(Error("Error. Invalid alarm"));
    }

    updatedEvent.alarm_id = alarmId;
    updatedEvent.updateFields |= calendar::ALARM_ID;
  }

  if (params->changes.description.get()) {
    updatedEvent.description = base::UTF8ToUTF16(*params->changes.description);
    updatedEvent.updateFields |= calendar::DESCRIPTION;
  }

  if (params->changes.title.get()) {
    updatedEvent.title = base::UTF8ToUTF16(*params->changes.title);
    updatedEvent.updateFields |= calendar::TITLE;
  }

  if (params->changes.start.get()) {
    double start = *params->changes.start;
    updatedEvent.start = GetTime(start);
    updatedEvent.updateFields |= calendar::START;
  }

  if (params->changes.end.get()) {
    double end = *params->changes.end;
    updatedEvent.end = GetTime(end);
    updatedEvent.updateFields |= calendar::END;
  }

  if (params->changes.all_day.get()) {
    updatedEvent.all_day = *params->changes.all_day;
    updatedEvent.updateFields |= calendar::ALLDAY;
  }

  if (params->changes.is_recurring.get()) {
    updatedEvent.is_recurring = *params->changes.is_recurring;
    updatedEvent.updateFields |= calendar::ISRECURRING;
  }

  if (params->changes.start_recurring.get()) {
    updatedEvent.start_recurring = GetTime(*params->changes.start_recurring);
    updatedEvent.updateFields |= calendar::STARTRECURRING;
  }

  if (params->changes.end_recurring.get()) {
    updatedEvent.end_recurring = GetTime(*params->changes.end_recurring);
    updatedEvent.updateFields |= calendar::ENDRECURRING;
  }

  if (params->changes.location.get()) {
    updatedEvent.location = base::UTF8ToUTF16(*params->changes.location);
    updatedEvent.updateFields |= calendar::LOCATION;
  }

  if (params->changes.url.get()) {
    updatedEvent.url = base::UTF8ToUTF16(*params->changes.url);
    updatedEvent.updateFields |= calendar::URL;
  }

  CalendarService* model = CalendarServiceFactory::GetForProfile(GetProfile());
  model->UpdateCalendarEvent(
      eventId, updatedEvent,
      base::Bind(&CalendarUpdateEventFunction::UpdateEventComplete, this),
      &task_tracker_);
  return RespondLater();  // UpdateEventComplete() will be called
                          // asynchronously.
}

void CalendarUpdateEventFunction::UpdateEventComplete(
    std::shared_ptr<calendar::UpdateEventResult> results) {
  if (!results->success) {
    Respond(Error("Error updating event"));
  } else {
    Respond(NoArguments());
  }
}

ExtensionFunction::ResponseAction CalendarDeleteEventFunction::Run() {
  std::unique_ptr<vivaldi::calendar::DeleteEvent::Params> params(
      vivaldi::calendar::DeleteEvent::Params::Create(*args_));

  EXTENSION_FUNCTION_VALIDATE(params.get());

  base::string16 id;
  id = base::UTF8ToUTF16(params->id);
  calendar::EventID eventId;

  if (!GetIdAsInt64(id, &eventId)) {
    return RespondNow(Error("Error. Invalid event id"));
  }

  CalendarService* model = CalendarServiceFactory::GetForProfile(GetProfile());

  model->DeleteCalendarEvent(
      eventId,
      base::Bind(&CalendarDeleteEventFunction::DeleteEventComplete, this),
      &task_tracker_);
  return RespondLater();  // DeleteEventComplete() will be called
                          // asynchronously.
}

void CalendarDeleteEventFunction::DeleteEventComplete(
    std::shared_ptr<calendar::DeleteEventResult> results) {
  if (!results->success) {
    Respond(Error("Error deleting event"));
  } else {
    Respond(NoArguments());
  }
}

std::unique_ptr<vivaldi::calendar::Calendar> CreateVivaldiCalendar(
    const calendar::CalendarResult& result) {
  std::unique_ptr<vivaldi::calendar::Calendar> calendar(
      new vivaldi::calendar::Calendar());

  calendar->id = result.id();
  calendar->name.reset(new std::string(base::UTF16ToUTF8(result.name())));

  calendar->description.reset(
      new std::string(base::UTF16ToUTF8(result.description())));
  calendar->url.reset(new std::string(result.url().spec()));
  calendar->orderindex.reset(new int(result.orderindex()));
  calendar->color.reset(new std::string(result.color()));
  calendar->hidden.reset(new bool(result.hidden()));

  return calendar;
}

ExtensionFunction::ResponseAction CalendarCreateFunction::Run() {
  std::unique_ptr<vivaldi::calendar::Create::Params> params(
      vivaldi::calendar::Create::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  calendar::CalendarRow createCalendar;

  base::string16 name;
  name = base::UTF8ToUTF16(params->calendar.name);

  createCalendar.set_name(name);

  base::string16 description;
  if (params->calendar.description.get()) {
    description = base::UTF8ToUTF16(*params->calendar.description);
    createCalendar.set_description(description);
  }

  std::string url;
  if (params->calendar.url.get()) {
    url = *params->calendar.url.get();
    createCalendar.set_url(GURL(url));
  }

  int orderindex;
  if (params->calendar.orderindex.get()) {
    orderindex = *params->calendar.orderindex.get();
    createCalendar.set_orderindex(orderindex);
  }

  std::string color;
  if (params->calendar.color.get()) {
    color = *params->calendar.color.get();
    createCalendar.set_color(color);
  }

  CalendarService* model = CalendarServiceFactory::GetForProfile(GetProfile());

  model->CreateCalendar(
      createCalendar, base::Bind(&CalendarCreateFunction::CreateComplete, this),
      &task_tracker_);
  return RespondLater();
}

void CalendarCreateFunction::CreateComplete(
    std::shared_ptr<calendar::CreateCalendarResult> results) {
  if (!results->success) {
    Respond(Error("Error creating calendar"));
  } else {
    Calendar ev = GetCalendarItem(results->createdRow);
    Respond(ArgumentList(
        extensions::vivaldi::calendar::Create::Results::Create(ev)));
  }
}

CalendarGetAllFunction::~CalendarGetAllFunction() {}

ExtensionFunction::ResponseAction CalendarGetAllFunction::Run() {
  CalendarService* model = CalendarServiceFactory::GetForProfile(GetProfile());

  model->GetAllCalendars(
      base::Bind(&CalendarGetAllFunction::GetAllComplete, this),
      &task_tracker_);

  return RespondLater();  // GetAllComplete() will be called
                          // asynchronously.
}

void CalendarGetAllFunction::GetAllComplete(
    std::shared_ptr<calendar::CalendarQueryResults> results) {
  CalendarList calendarList;

  if (results && !results->empty()) {
    for (calendar::CalendarQueryResults::CalendarResultVector::const_iterator
             iterator = results->begin();
         iterator != results->end(); ++iterator) {
      calendarList.push_back(std::move(
          *base::WrapUnique((CreateVivaldiCalendar(*iterator).release()))));
    }
  }

  Respond(
      ArgumentList(vivaldi::calendar::GetAll::Results::Create(calendarList)));
}

ExtensionFunction::ResponseAction CalendarUpdateFunction::Run() {
  std::unique_ptr<vivaldi::calendar::Update::Params> params(
      vivaldi::calendar::Update::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  calendar::Calendar updatedCalendar;

  base::string16 id;
  id = base::UTF8ToUTF16(params->id);
  calendar::CalendarID calendarId;
  if (!GetIdAsInt64(id, &calendarId)) {
    return RespondNow(Error("Error. Invalid calendar id"));
  }

  if (params->changes.name.get()) {
    updatedCalendar.name = base::UTF8ToUTF16(*params->changes.name);
    updatedCalendar.updateFields |= calendar::CALENDAR_NAME;
  }

  if (params->changes.description.get()) {
    updatedCalendar.description =
        base::UTF8ToUTF16(*params->changes.description);
    updatedCalendar.updateFields |= calendar::CALENDAR_DESCRIPTION;
  }

  if (params->changes.url.get()) {
    updatedCalendar.url = GURL(*params->changes.url);
    updatedCalendar.updateFields |= calendar::CALENDAR_URL;
  }

  if (params->changes.orderindex.get()) {
    updatedCalendar.orderindex = *params->changes.orderindex;
    updatedCalendar.updateFields |= calendar::CALENDAR_ORDERINDEX;
  }

  if (params->changes.color.get()) {
    updatedCalendar.color = *params->changes.color;
    updatedCalendar.updateFields |= calendar::CALENDAR_COLOR;
  }

  if (params->changes.hidden.get()) {
    updatedCalendar.hidden = *params->changes.hidden;
    updatedCalendar.updateFields |= calendar::CALENDAR_HIDDEN;
  }

  CalendarService* model = CalendarServiceFactory::GetForProfile(GetProfile());
  model->UpdateCalendar(
      calendarId, updatedCalendar,
      base::Bind(&CalendarUpdateFunction::UpdateCalendarComplete, this),
      &task_tracker_);
  return RespondLater();  // UpdateCalendarComplete() will be called
                          // asynchronously.
}

void CalendarUpdateFunction::UpdateCalendarComplete(
    std::shared_ptr<calendar::UpdateCalendarResult> results) {
  if (!results->success) {
    Respond(Error("Error updating calendar"));
  } else {
    Respond(NoArguments());
  }
}

ExtensionFunction::ResponseAction CalendarDeleteFunction::Run() {
  std::unique_ptr<vivaldi::calendar::Delete::Params> params(
      vivaldi::calendar::Delete::Params::Create(*args_));

  EXTENSION_FUNCTION_VALIDATE(params.get());

  base::string16 id;
  id = base::UTF8ToUTF16(params->id);
  calendar::CalendarID calendarId;

  if (!GetIdAsInt64(id, &calendarId)) {
    return RespondNow(Error("Error. Invalid calendar id"));
  }

  CalendarService* model = CalendarServiceFactory::GetForProfile(GetProfile());

  model->DeleteCalendar(
      calendarId,
      base::Bind(&CalendarDeleteFunction::DeleteCalendarComplete, this),
      &task_tracker_);
  return RespondLater();  // DeleteCalendarComplete() will be called
                          // asynchronously.
}

void CalendarDeleteFunction::DeleteCalendarComplete(
    std::shared_ptr<calendar::DeleteCalendarResult> results) {
  if (!results->success) {
    Respond(Error("Error deleting calendar"));
  } else {
    Respond(NoArguments());
  }
}

}  //  namespace extensions
