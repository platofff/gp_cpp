/* global Module, postMessage */
let eventQueue = []

const methods = {
  /**
   * Runs the pattern generator and sends the result back to main thread
   * @param {Array<Array<ImageData>>} collections
   * @param {Number} canvasWidth
   * @param {Number} canvasHeight
   * @param {Number} threshold
   * @param {Number} offsetRadius
   * @param {Number} collectionOffsetRadius
   */
  run(collections, canvasWidth, canvasHeight, threshold, offsetRadius, collectionOffsetRadius) {
    const collectionsV = new Module.CollectionVector()

    for (const c of collections) {
      const imagesV = new Module.Collection()

      for (const img of c) {
        let data = new Module.Uint8Vector()
        for (let i = 0; i < img.data.length; i += 4) {
          data.push_back(img.data[i + 3])
        }

        const imgAlpha = new Module.ImgAlpha(data, img.width, img.height, threshold)
        imagesV.push_back(imgAlpha)
      }

      collectionsV.push_back(imagesV)
    }

    const patternGenerator = new Module.PatternGenerator(canvasWidth, canvasHeight, collectionsV, offsetRadius, collectionOffsetRadius)
    const resultV = patternGenerator.generate()
    
    const result = []

    for (let i = 0; i < resultV.size(); i++) {
      const collection = []
      const collectionV = resultV.get(i)
      for (let j = 0; j < collectionV.size(); j++) {
        const offsets = []
        const offsetsV = collectionV.get(j)
        for (let k = 0; k < offsetsV.size(); k++) {
          const offset = offsetsV.get(k)
          offsets.push({ x: offset.x, y: offset.y })
        }
        collection.push(offsets)
      }
      result.push(collection)
    }

    postMessage({ result: result })
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
})
