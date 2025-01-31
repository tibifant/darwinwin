const server_url = 'http://localhost:21110/';

//Cached Variables
let actorElements = [];
let worldData;
let aiStepIntervalIdCache;

//Assistant Variables
let currentlySelectedTileID = null;

const tileFlags = {
      Underwater: 1 << 0,
      Protein: 1 << 1,
      Sugar: 1 << 2,
      Vitamin: 1 << 3,
      Fat: 1 << 4,
      Collidable: 1 << 5,
      OtherActor: 1 << 6,
      Hidden: 1 << 7,
    }

let mapTiles = [];

const emptyColor = "rgb(112, 168, 87)";

document.addEventListener('DOMContentLoaded', setup);

// ### Initial Setup Functions ###
function setup(){
  console.log("Initiating Setup...");
  setupControlPanel();
  fetchLevel(setupMap);
  setupStatsWindow();
  setupViewCone();
  console.log("Setup Complete!");
}

function setupControlPanel(){
}

function setupStatsWindow(){
  const infoLabelsElement = document.getElementById("stats-info-labels");
  const optionsElement = document.getElementById("stats-options");

  let statsMessage = document.createElement("label")
  statsMessage.innerText = "Click on tile or actor to reveal stats..."
  infoLabelsElement.appendChild(statsMessage);
  statsMessage = document.createElement("label")
  statsMessage.innerText = "Click on tile or actor to see options..."
  optionsElement.appendChild(statsMessage);
}

function setupMap(){
  const levelContainer = document.getElementById("level-container");
  setLevelContainerSize(levelContainer);
  setupTileElements(levelContainer, worldData.level.grid);

  worldData.actor.forEach((actor, i) => {
    setupActor(actor, i, worldData.level.width);
  })

    updateWorld();
}

function setLevelContainerSize(levelContainer){
  //Adjust according to level-tile css class
  const tileSize = 20;
  const tileMargin = 1;
  const containerWidth = worldData.level.height * (tileSize + tileMargin*4) + 'pt';
  const containerHeight = worldData.level.width * (tileSize + tileMargin*4) + 'pt';

  levelContainer.style.width = containerWidth;
  levelContainer.style.height = containerHeight;

  levelContainer.style.minWidth = containerWidth;
  levelContainer.style.minHeight = containerHeight;
}

function setupTileElements(levelContainer, grid){
  grid.forEach((tile, i) => {
    const newTile = document.createElement("div");
    newTile.classList.add("level-tile");
    newTile.id = "tile-"+i;
    newTile.addEventListener("click", event => {
      showTileStats(event.target.id);
      handleCurrentlySelectedTile(document.getElementById(event.target.id));
    })
    levelContainer.appendChild(newTile);
  })
}

function handleCurrentlySelectedTile(currentlySelectedTile){
  if(currentlySelectedTileID){
    const previousSelectedTile = document.getElementById(currentlySelectedTileID);
    previousSelectedTile.classList.remove("selected-tile");
  }
  if(currentlySelectedTile){
    currentlySelectedTile.classList.add("selected-tile");
    currentlySelectedTileID = event.target.id;
  }else{
    currentlySelectedTileID = null;
  }
  console.log("Currently selected tile: ", currentlySelectedTileID);
}

function setupActor(actor, index){
  const newActorElement = document.createElement("div");
  newActorElement.classList.add("actor");
  newActorElement.id = "actor-"+index;
  newActorElement.addEventListener("click", event => {
    showActorStats(event.target.id);
    event.stopPropagation();
  })
  actorElements[index] = newActorElement;

  updateActor(actor, index);
}

function setupViewCone(){
  //TODO: Possible to make this more dynamic?
  const viewConeSize = 8;
  const positions = [
    { row: 1, col: 2 },
    { row: 2, col: 1 },
    { row: 2, col: 2 },
    { row: 2, col: 3 },
    { row: 3, col: 1 },
    { row: 3, col: 2 },
    { row: 3, col: 3 },
    { row: 4, col: 2 },
  ];

  const view_cone_grids = [document.getElementById('view-cone-grid-0'), document.getElementById('view-cone-grid-1')] || [];

  for (let i = 0; i < view_cone_grids.length; i++) {
    const grid = view_cone_grids[i];
    if (grid.length === 0) {
      console.error("Error: Could not find view-cone-grid element");
    } else {
      for (let j = 0; j < viewConeSize; j++) {
        const tile = document.createElement('div');
        tile.classList.add('view-cone-tile');
        tile.style.gridRow = positions[j].row;
        tile.style.gridColumn = positions[j].col;
        tile.id = 'view-cone-tile-' + i + "-" + j;
        grid.appendChild(tile);
      }
    }
  }
}

// ### Periodic Update Functions ###
function fetchAllData(callback = null) {
  console.log("Updating World...");
  //Tried to avoid lambda but could not find alternative
  fetchLevel(res => {
    updateWorld(callback);
  });
}

function updateWorld(callback){
  console.log("Received World Data:\n", worldData);

  updateTiles();

  worldData.actor.forEach((actor, i) => {
      updateActor(actor, i);
  });
  if (callback) callback();
}

function updateTiles(){
  const grid = worldData.level.grid;
  for(let i=0; i<grid.length; i++){
    const tileElement = document.getElementById("tile-"+i);
    tileElement.classList.remove("view-cone");

    if(!mapTiles || mapTiles[i] !== grid[i]) {
      console.log("Tile-" + i + " has changed from value " + mapTiles[i] + " to " + grid[i]);
      checkTileFlags(grid[i], tileElement, i);
    }
  }
}

function checkTileFlags(tile, tileElement, tileIndex){
  let hasFoodCondition = false;
  tileElement.classList.forEach(cssClass => tileElement.classList.remove(cssClass));
  tileElement.style.backgroundColor = '';
  tileElement.classList.add('level-tile');

  if(hasTileCondition(tile, "Hidden")){
    if(tileElement.id.split('-')[0] === 'view'){
      loadIcon(tileElement, 'hidden1', null, 1, null);
      if(tileIndex){
        mapTiles[tileIndex] = tile;
      }
      return;
    }
  }
  if(hasTileCondition(tile, "OtherActor")){

  }
  if(hasTileCondition(tile, "Collidable")){
    loadIcon(tileElement, 'collidable1', null, 1, null);
    return;
  }
  if(hasTileCondition(tile, "Fat")){
    hasFoodCondition = true;
    loadIcon(tileElement, 'fat1', null, 1, null);
  }
  if(hasTileCondition(tile, "Vitamin")){
    hasFoodCondition = true;
    loadIcon(tileElement, 'vitamin1', null, 1, null);
  }
  if(hasTileCondition(tile, "Sugar")){
    hasFoodCondition = true;
    loadIcon(tileElement, 'sugar1', null, 1, null);
  }
  if(hasTileCondition(tile, "Protein")){
    hasFoodCondition = true;
    loadIcon(tileElement, 'protein1', null, 1, null);
  }
  if(hasTileCondition(tile, "Underwater")){
    if(!hasFoodCondition){
      loadIcon(tileElement, 'underwater1', null, 0.3, '#16829E');
    }else{
      tileElement.style.backgroundColor = '#16829E';
    }
  }
  else{
    if(!hasFoodCondition) {
      loadIcon(tileElement, 'noFlag1', null, 0.4, null);
    }
  }

  if(tileIndex !== null){
    console.log("Updating tile " + tileIndex + " to value " + tile);
    mapTiles[tileIndex] = tile;
  }
}

function hasTileCondition(tile, condition){
  return !!(tile & tileFlags[condition]);
}

function loadIcon(tileElement, icon1, icon2, distribution, colorCode){
  const randomizer = Math.random();
  if(icon2 && randomizer > 1 - distribution/2){
    tileElement.classList.add('icon', icon2);
  }else if(randomizer < distribution/2 || !icon2 && randomizer < distribution){
    tileElement.classList.add( 'icon', icon1);
  }else{
    tileElement.style.backgroundColor = colorCode || null;
  }
}

function updateActor(actor, index){
  console.log(worldData)
  const actorTile = document.getElementById("tile-" + (actor.posX + worldData.level.width * actor.posY));
  actorTile.appendChild(actorElements[index]);
  worldData.actor[index] = actor;
  if(actor.stats[5] === 0){
    actorElements[index].classList.add('dead');
  }else{
    actorElements[index].classList.remove('dead');
    drawViewConeOnMap(index);
  }
}

// ### Stats view for Actor functions ###
function showActorStats(actorId){
  console.log("Showing Actor Stats for " + actorId);
  const infoLabelsElement = document.getElementById("stats-info-labels");
  const infoValuesElement = document.getElementById("stats-info-values");
  const optionsElement = document.getElementById("stats-options");
  infoLabelsElement.innerHTML = '';
  infoValuesElement.innerHTML = '';
  optionsElement.innerHTML = '';

  const labelsElement = document.createElement('label')
  const valuesElement = document.createElement('label');

  fillActorLabelsAndValuesElements(actorId, labelsElement, valuesElement);

  infoLabelsElement.appendChild(labelsElement);
  infoValuesElement.appendChild(valuesElement);
  optionsElement.appendChild(createActorOptionsButtonsElement(actorId));

  const actorIndex = actorId.split('-')[1];
  showBackendViewCone(actorIndex);
  showFrontendViewCone(calculateViewConeTileIndexes(actorIndex));
  handleCurrentlySelectedTile(null);
}

function showBackendViewCone(actorIndex){
  const viewCone = worldData.actor[actorIndex].viewcone;

  for (let i = 0; i < viewCone.length; i++) {
    const tile = document.getElementById('view-cone-tile-' + 0 + "-" + i);
    tile.innerText = viewCone[i];
    tile.style.backgroundColor = emptyColor;
    checkTileFlags(viewCone[i], tile, null);
    //tile.style.height = 'auto';
    //tile.style.width = 'auto';
  }
  displayActorIcon(document.getElementById('view-cone-tile-0-0'));
}

function calculateViewConeTileIndexes(actorIndex){
  const lookDir = worldData.actor[actorIndex].lookDir;
  const mapWidth = worldData.level.width;
  const actorTileIndex = worldData.actor[actorIndex].posY * mapWidth + worldData.actor[actorIndex].posX;
  switch (lookDir) {
    case 0:
      return [
        actorTileIndex,
        actorTileIndex + mapWidth - 1,
        actorTileIndex - 1,
        actorTileIndex - mapWidth - 1,
        actorTileIndex + mapWidth - 2,
        actorTileIndex - 2,
        actorTileIndex - mapWidth - 2,
        actorTileIndex - 3
      ];
    case 1:
      return [
        actorTileIndex,
        actorTileIndex - mapWidth - 1,
        actorTileIndex - mapWidth,
        actorTileIndex - mapWidth + 1,
        actorTileIndex - 2 * mapWidth - 1,
        actorTileIndex - 2 * mapWidth,
        actorTileIndex - 2 * mapWidth + 1,
        actorTileIndex - 3 * mapWidth
      ];
    case 2:
      return [
        actorTileIndex,
        actorTileIndex - mapWidth + 1,
        actorTileIndex + 1,
        actorTileIndex + mapWidth + 1,
        actorTileIndex - mapWidth + 2,
        actorTileIndex + 2,
        actorTileIndex + mapWidth + 2,
        actorTileIndex + 3
      ];
    case 3:
      return [
        actorTileIndex,
        actorTileIndex + mapWidth + 1,
        actorTileIndex + mapWidth,
        actorTileIndex + mapWidth - 1,
        actorTileIndex + 2 * mapWidth + 1,
        actorTileIndex + 2 * mapWidth,
        actorTileIndex + 2 * mapWidth - 1,
        actorTileIndex + 3 * mapWidth
      ];
    default:
      console.error("Error: Invalid lookDir -> function: calculateViewConeTileIndexes");
  }
}

function showFrontendViewCone(tileIndexes){
  tileIndexes.forEach((index, i) => {
    const gridTile = document.getElementById('view-cone-tile-'+ 1 + "-" + i);
    const tileValue = worldData.level.grid[index];
    gridTile.style.backgroundColor = emptyColor;
    gridTile.innerText = tileValue;
    checkTileFlags(tileValue, gridTile, null);
  })
  displayActorIcon(document.getElementById('view-cone-tile-1-0'));
}

function displayActorIcon(actorTile){
  const icon = document.createElement("div");
  icon.className = "actor-icon";
  actorTile.appendChild(icon);
}

function drawViewConeOnMap(index){
  const viewConeTileIndexes = calculateViewConeTileIndexes(index);
  viewConeTileIndexes.forEach((tileIndex) => {
    const tile = document.getElementById('tile-'+tileIndex);
    if(tile === null){
      console.error("Error: Could not find tile element -> function: drawViewConeOnMap");
    }else{
      tile.classList.add('level-tile', 'view-cone');
    }
  })

}

function fillActorLabelsAndValuesElements(actorId, labels, values){
  const lookDirections = [
    "left",
    "up",
    "right",
    "down",
  ]
  const statDescriptions = [
    "Air",
    "Protein",
    "Sugar",
    "Vitamin",
    "Fat",
    "Energy"
  ]

  const actorIndex = actorId.split('-')[1];
  const targetActorStats = worldData.actor[actorIndex];
  labels.innerHTML = "Actor ID:<br>PosX:<br>PosY:<br>LookDir:<br><br>";
  values.innerHTML = `${actorId}<br>${targetActorStats.posX}<br>${targetActorStats.posY}<br>${lookDirections[targetActorStats.lookDir]}<br><br>`;
  for (const stat in targetActorStats.stats ){
    labels.innerHTML += statDescriptions[stat] + "<br>";
    values.innerHTML += targetActorStats.stats[stat] + "<br>";
  }
}

function createActorOptionsButtonsElement(actorId){
  const actorActions = [
    "move",
    "move 2",
    "turn left",
    "turn right",
    "eat",
    "wait",
    "move diagonal-left",
    "move diagonal-right"
  ]

  const optionsButtonsElement = document.createElement('div');
  optionsButtonsElement.classList.add('stats-subsection-column');
  actorActions.forEach((action, i) => {
    const button = createActorButton(actorId, action, i);
    optionsButtonsElement.appendChild(button);
  })
  return optionsButtonsElement;
}

function createActorButton(actorId, action, actionId){
  const button = document.createElement('button');
  button.id = "option-button-"+actorId+"-"+actionId;
  button.innerText = action;
  button.addEventListener('click', initiateActorActionRequest)
  return button;
}

// ### Stats view for Tile functions ###
function showTileStats(tileId){
  const infoLabelsElement = document.getElementById("stats-info-labels");
  const infoValuesElement = document.getElementById("stats-info-values");
  const optionsElement = document.getElementById("stats-options");
  infoLabelsElement.innerHTML = '';
  infoValuesElement.innerHTML = '';
  optionsElement.innerHTML = '';

  const optionsButtonsElement = document.createElement('div');
  optionsButtonsElement.classList.add('stats-subsection-column');
  const labels = document.createElement('label');
  const values = document.createElement('value');

  fillTileStatsElements(tileId, labels, values, optionsButtonsElement);

  infoLabelsElement.appendChild(labels);
  infoValuesElement.appendChild(values);
  optionsElement.appendChild(optionsButtonsElement);
}

function fillTileStatsElements(id, labels, values, optionsButtonsElement){
  const tileIndex = id.split('-')[1];
  const x = tileIndex % worldData.level.width;
  const y = Math.floor(tileIndex / worldData.level.width);

  labels.innerHTML = "Tile ID:<br>X:<br>Y:<br><br>";
  values.innerHTML = `${id}<br>${x}<br>${y}<br><br>`;

  const grid = worldData.level.grid;

  //TODO: Remove <br> and use CSS for spacing?
  for(const flag in tileFlags){
    labels.innerHTML += flag + "<br>";

    const checkbox = document.createElement('input');
    checkbox.type = 'checkbox';
    checkbox.id = `checkbox-${flag}`;
    checkbox.checked = hasTileCondition(grid[tileIndex], flag);

    checkbox.addEventListener('change', () => {
      //fyi: flag wird hier nicht durch den loop überschrieben, das Prinzip nennt man closure
      console.log("Sending request for flag " + flag + " to tile " + currentlySelectedTileID);
      postTileSetRequest(getPayload(flag), currentlySelectedTileID);
    });

    values.appendChild(checkbox);
    values.appendChild(document.createElement('br'));
  }
}

function getPayload(flag){
  const tileIndex = currentlySelectedTileID.split('-')[1];
  const tile = worldData.level.grid[tileIndex];
  const x = tileIndex % worldData.level.width;
  const y = Math.floor(tileIndex / worldData.level.width);

  const newValue = tile ^ tileFlags[flag];

  return {
    x: x,
    y: y,
    value: newValue
  }
}

// ### Control Panel functions ###

function levelGenerate(){
  postGenerateLevelRequest();
}

function reloadBrain(){
  postLoadBrainRequest();
}

function resetStats(){
  postResetStatsRequest();
}

function aiStep(){
  postAiStepRequest();
}

function aiStepToggle(event){
  const element = event.target;
  switch (element.id.split('-')[2]) {
    case 'start':
      aiStepIntervalIdCache = setInterval(aiStep, 1000);
      element.id = "ai-step-stop-button";
      element.innerText = "Stop AI Step";
      break;
    case 'stop':
      if(!aiStepIntervalIdCache){
        console.error("Stop AI Step interval was called before interval was started.")
      }
      clearInterval(aiStepIntervalIdCache);
      element.id = "ai-step-start-button";
      element.innerText = "Start AI Step";
      break;
    default:
      console.error("StepToggle - ID not recognized: " + element.id);
  }
}

function loadTrainingLevel(){
  postLoadTrainingLevelRequest();
}

//TODO: Merge Buttons into single one, that switches function based on state
function startTraining(){
  postTrainingStartRequest();
}

function stopTraining(){
  postTrainingStopRequest();
}

// ### Set functions ###
function initiateActorActionRequest(event){
  const idSplit = event.target.id.split('-');
  const actorId = idSplit[2] + "-" + idSplit[3];
  const actionId = idSplit[4];
  postActorAction(actorId, actionId);
}

// ### Net functions ###
function handleError(error){
  console.error("Encountered error during fetch:\n", error);
}

function load_url(url, callback, payload, failure_callback) {
  var xmlhttp;

  console.log("Sending request to '" + url + "':");
  //console.log(payload);

  if (window.XMLHttpRequest)
    xmlhttp = new XMLHttpRequest();
  else
    xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");

  xmlhttp.onreadystatechange = function () {
    if (xmlhttp.readyState == 4) {
      if (xmlhttp.status >= 200 && xmlhttp.status < 300) {
        try {
          let obj = JSON.parse(xmlhttp.responseText);
          callback(obj);
        } catch (e) {
          if(e instanceof SyntaxError) {
            console.log("Request returned Faulty JSON, attempting callback without additional parameters.");
            callback();
          }
          else {
            failure_callback(e);
          }
        }
      } else { // TODO: handle all relevant error codes
        if (xmlhttp.status == 403) {

        } else {
          failure_callback(false);
        }
      }
    }
  }

  xmlhttp.ontimeout = (e) => {
    failure_callback();
  };

  xmlhttp.timeout = 7500;
  xmlhttp.open("POST", url, true);
  xmlhttp.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
  xmlhttp.send(JSON.stringify(payload));
}

function load_backend_url(url, callback, payload, failure_callback) {
  load_url(server_url + url, callback, payload, failure_callback);
}

function fetchLevel(callback){
   load_backend_url('getLevel', res => {
      worldData = res;
      callback(worldData);
     },
    {}, handleError);
}

function fetchTrainingState(callback){
  load_backend_url('is_training', callback, {}, handleError)
}

function postActorAction(actorId, actionId) {
  const updateFunction = showActorStats.bind(null, actorId);
  const fetchAndUpdate = fetchAllData.bind(null, updateFunction);
  load_backend_url('manualAct', fetchAndUpdate,
    {actionId: actionId}, handleError);
}

function postTileSetRequest(payload, tileId){
  const updateFunction = showTileStats.bind(null, tileId);
  const fetchAndUpdate = fetchAllData.bind(null, updateFunction);
  load_backend_url('setTile', fetchAndUpdate,
    payload, handleError);
}

function postGenerateLevelRequest(){
  load_backend_url('level_generate', fetchAllData, {})
}

function postResetStatsRequest(){
  load_backend_url('reset_stats', fetchAllData, {}, handleError);
}

function postAiStepRequest(){
  load_backend_url('ai_step', fetchAllData, {}, handleError);
}

function postLoadBrainRequest(){
  load_backend_url('load_training_actor', fetchAllData, {}, handleError);
}

function postLoadTrainingLevelRequest(){
  load_backend_url('load_training_level', fetchAllData, {}, handleError);
}

function postTrainingStartRequest(){
  load_backend_url('start_training', {}, {}, handleError);
}

function postTrainingStopRequest(){
  load_backend_url('stop_training', {}, {}, handleError);
}
