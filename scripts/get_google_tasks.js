function doGet(e) {
  try {
    // 1. Extract IDs from the request URL
    var idsParam = e && e.parameter ? e.parameter.ids : null;
    var targetListIds = [];

    if (idsParam) {
      targetListIds = idsParam.split(',').map(function(id) {
        return id.trim();
      });
    }

    var allTasks = [];

    // 2. Route processing based on whether specific IDs were passed
    if (targetListIds.length > 0) {
      // Fetch only requested lists
      targetListIds.forEach(function(listId) {
        try {
          var tasks = fetchAllTasksForList(listId);
          var filteredTasks = tasks.map(function(task) {
            return {
              due: task.due,
              title: task.title,
              status: task.status
            };
          });
          allTasks = allTasks.concat(filteredTasks);
        } catch (err) {
          // Skip lists that fail to fetch; log for debugging if needed
          Logger.log("Failed to fetch list " + listId + ": " + err.message);
        }
      });
    } else {
      // Fallback: Fetch ALL lists if no 'ids' parameter is provided
      var taskListsResponse = Tasks.Tasklists.list({ maxResults: 100 });
      var taskLists = taskListsResponse.items || [];

      taskLists.forEach(function(list) {
        var tasks = fetchAllTasksForList(list.id);

        // Map the tasks to extract only the required fields
        var filteredTasks = tasks.map(function(task) {
          return {
            due: task.due,
            title: task.title,
            status: task.status
          };
        });

        allTasks = allTasks.concat(filteredTasks);
      });
    }

    // 3. Sort all tasks chronologically by due date (tasks with no due date go last)
    allTasks.sort(function(a, b) {
      if (!a.due && !b.due) return 0;
      if (!a.due) return 1;
      if (!b.due) return -1;
      return new Date(a.due) - new Date(b.due);
    });

    // 4. Return JSON with appropriate CORS headers (plain array of tasks)
    return ContentService.createTextOutput(JSON.stringify(allTasks))
                         .setMimeType(ContentService.MimeType.JSON);

  } catch (error) {
    // Return graceful JSON error wrapper
    return ContentService.createTextOutput(JSON.stringify({ status: "error", message: error.toString() }))
                         .setMimeType(ContentService.MimeType.JSON);
  }
}

/**
 * Helper to fetch tasks from a specific task list
 */
function fetchAllTasksForList(listId) {
  var tasksResponse = Tasks.Tasks.list(listId, {
    maxResults: 100,
    showCompleted: true,
    showHidden: true
  });
  return tasksResponse.items || [];
}