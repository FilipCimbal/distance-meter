// eslint-disable-next-line @typescript-eslint/no-var-requires
const fs = require('fs')
// eslint-disable-next-line @typescript-eslint/no-var-requires
const {exec} = require('child_process')

exec('git describe --always', (error, stdout, stderr) => {
    fs.writeFileSync('build.json', JSON.stringify({buildInfo:{
        datetime: new Date(),
        hash: stdout.trim()
    }}))
})
