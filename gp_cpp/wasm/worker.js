/* global Module, postMessage */
let resOptimizer = null
let eventQueue = []

/** @type {Array<Array<ImageData>>} */
let collections = []

const methods = {
  createCollection() {
    collections.push([])
  },

  /**
   * Add image to collection
   * @param {Number} collectionIdx
   * @param {ImageData} image
   */
  addImage(collectionIdx, image) {
    collections[collectionIdx].push(image)
  },

  clear() {
    collections = []
  },

  /**
   * Runs the pattern generator and sends the result back to main thread
   * @param {Number} canvasWidth
   * @param {Number} canvasHeight
   * @param {Number} threshold
   * @param {Number} offsetRadius
   * @param {Number} collectionOffsetRadius
   */
  run(canvasWidth, canvasHeight, threshold, offsetRadius, collectionOffsetRadius) {
    const collectionsV = new Module.CollectionVector();
  
    for (const c of collections) {
      const imagesV = new Module.Collection();
  
      for (const img of c) {
        let data = new Module.Uint8Vector();
        for (let i = 0; i < img.data.length; i += 4) {
          data.push_back(img.data[i + 3])
        }
  
        const imgAlpha = new Module.ImgAlpha(data, img.width, img.height, threshold)
        imagesV.push_back(imgAlpha)
      }
  
      collectionsV.push_back(imagesV);
    }
  
    const patternGenerator = new Module.PatternGenerator(canvasWidth, canvasHeight, collectionsV, offsetRadius, collectionOffsetRadius);
    const result = patternGenerator.generate();
    console.log(result)

    postMessage({ patternResult: result });
  }

}

const processEvent = (e) => {
  methods[e.data.method](...e.data.params)
}

Module.onRuntimeInitialized = () => {
  for (const e of eventQueue) {
    processEvent(e);
  }
  eventQueue = [];
}

self.addEventListener('message', (e) => {
  if (Module.calledRun) {
    processEvent(e)
  } else {
    // Module is not ready, queue the event
    eventQueue.push(e)
  }
});
